#include "UIMapWindow.h"
#include "cavepacker/client/ui/nodes/UINodeMap.h"
#include "cavepacker/client/ui/nodes/UICavePackerNodePoint.h"
#include "cavepacker/shared/network/messages/MoveToMessage.h"
#include "ui/nodes/UINodeBar.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodePoint.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeMapOnScreenCursorControl.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/UI.h"
#include "client/Camera.h"
#include "common/ConfigManager.h"
#include "service/ServiceProvider.h"
#include "common/Log.h"
#include "common/IFrontend.h"

namespace cavepacker {

class SolveListener: public UINodeListener {
private:
	UINodeSlider *_sliderNode;
	std::string _configVar;
public:
	SolveListener (UINodeSlider *sliderNode, const std::string& configVar) :
			_sliderNode(sliderNode), _configVar(configVar)
	{
		_sliderNode->setValue(Config.getConfigVar(_configVar)->getFloatValue());
	}

	void onValueChanged () override
	{
		const float val = _sliderNode->getValue();
		Config.getConfigVar(_configVar)->setValue(string::toString(val));
	}
};

UIMapWindow::UIMapWindow (IFrontend *frontend, ServiceProvider& serviceProvider, CampaignManager& campaignManager, CavePackerClientMap& map) :
		IUIMapWindow(frontend, serviceProvider, campaignManager, map,
				new UINodeMap(frontend, serviceProvider, campaignManager, 0, 0,
						frontend->getWidth(), frontend->getHeight(), map)), _undo(
				nullptr), _points(nullptr), _campaignManager(campaignManager), _scrolling(false), _targetX(-1), _targetY(-1) {
	init();

	const float height = 0.05f;
	const float sliderWidth = 0.3f;
	_autoSolveSlider = new UINodeSlider(frontend, 10.0f, 1000.0f, 10.0f);
	_autoSolveSlider->setAlignment(NODE_ALIGN_BOTTOM | NODE_ALIGN_CENTER);
	_autoSolveSlider->setSize(sliderWidth, height);
	_autoSolveSlider->addListener(UINodeListenerPtr(new SolveListener(_autoSolveSlider, "solvestepmillis")));
	_autoSolveSlider->setId("autosolveslider");
	add(_autoSolveSlider);
	hideAutoSolveSlider();
}

void UIMapWindow::showAutoSolveSlider()
{
	_autoSolveSlider->setVisible(true);
}

void UIMapWindow::hideAutoSolveSlider()
{
	_autoSolveSlider->setVisible(false);
}

void UIMapWindow::showCursor (bool /*show*/)
{
	IUIMapWindow::showCursor(true);
}

void UIMapWindow::hideHud()
{
	IUIMapWindow::hideHud();
	_undo->setVisible(false);
}

void UIMapWindow::showHud()
{
	IUIMapWindow::showHud();
	_undo->setVisible(true);
	hideAutoSolveSlider();
}

void UIMapWindow::initHudNodes ()
{
	UINode* panel = new UINode(_frontend);
	panel->setId("hudpanel_bonus");
	panel->setImage("bones");
	panel->setStandardPadding();
	panel->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_CENTER);
	add(panel);

	UINode *innerPanel = new UINode(_frontend);
	innerPanel->setId("hudpanel_inner");
	innerPanel->setLayout(new UIHBoxLayout(0.01f));
	innerPanel->setPos(panel->getX() + 0.07f, panel->getY());
	_points = new UICavePackerNodePoint(_frontend, 30);
	_points->setFont(HUGE_FONT);
	_points->setId(UINODE_POINTS);
	innerPanel->add(_points);
	add(innerPanel);
}

void UIMapWindow::initInputHudNodes ()
{
	IUIMapWindow::initInputHudNodes();
	_undo = new UINodeButton(_frontend);
	_undo->setId("undo");
	_undo->setImage("icon-undo");
	_undo->setAlignment(NODE_ALIGN_TOP | NODE_ALIGN_RIGHT);
	_undo->setOnActivate("undo");
	add(_undo);
}

UINode* UIMapWindow::getFingerControl ()
{
	UINodeMapOnScreenCursorControl* node = new UINodeMapOnScreenCursorControl(_frontend, _nodeMap);
	_mapControl = node;
	return node;
}

void UIMapWindow::initWaitingForPlayers (bool adminOptions) {
	IUIMapWindow::initWaitingForPlayers(adminOptions);
	ClientMap& map = _nodeMap->getMap();
	const std::string& name = map.getName();
	const CampaignPtr& c = _campaignManager.getActiveCampaign();
	const CampaignMap* campaignMap = c->getMapById(name);
	const int ownBest = campaignMap != nullptr ? campaignMap->getFinishPoints() : 0;
	const std::string best = map.getSetting("best", "0" /* string::toString(ownBest) */);
	Log::info(LOG_UI, "got best points from server: %s", best.c_str());
	_points->setOwnAndGlobalBest(ownBest, string::toInt(best));
	_points->setLabel("0");

	if (!_serviceProvider.getNetwork().isMultiplayer() && _campaignManager.firstMap())
		UI::get().push("introgame");
}

bool UIMapWindow::onFingerMotion (int64_t finger, uint16_t x, uint16_t y, int16_t dx, int16_t dy)
{
	if (IUIMapWindow::onFingerMotion(finger, x, y, dx, dy))
		return true;
	Camera& camera = _nodeMap->getMap().getCamera();
	camera.scroll(dx, dy);
	return true;
}

bool UIMapWindow::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	if (!IUIMapWindow::onMouseButtonRelease(x, y, button)) {
		_scrolling = false;
	}
	return true;
}

bool UIMapWindow::getField (int32_t x, int32_t y, int *tx, int *ty) const
{
	_nodeMap->getMap().getMapGridForScreenPixel(x, y, tx, ty);
	if (*tx == -1 || *ty == -1)
		return false;
	return true;
}

void UIMapWindow::doMove (int tx, int ty)
{
	Log::debug(LOG_UI, "send walk message to reach %i:%i", tx, ty);
	_serviceProvider.getNetwork().sendToServer(MoveToMessage(tx, ty));
	_targetX = _targetY = -1;
}

bool UIMapWindow::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	const bool retVal = IUIMapWindow::onMouseButtonPress(x, y, button);
	if (!retVal) {
		_scrolling = true;
	}
	if (button == SDL_BUTTON_LEFT) {
		int tx, ty;
		if (getField(x, y, &tx, &ty)) {
			// double clicking onto the same field means, that the users wanna walk there
			Log::debug(LOG_UI, "resolved the grid coordinates for %i:%i to %i:%i", x, y, tx, ty);
			if (_targetX == tx && _targetY == ty) {
				doMove(_targetX, _targetY);
			} else {
				_targetX = tx;
				_targetY = ty;
			}
			return true;
		}
		Log::error(LOG_UI, "could not get grid coordinates for %i:%i", x, y);
	}
	return retVal;
}

void UIMapWindow::onMouseMotion (int32_t x, int32_t y, int32_t relX, int32_t relY)
{
	if (_scrolling) {
		Camera& camera = _nodeMap->getMap().getCamera();
		camera.scroll(relX, relY);
	}
	IUIMapWindow::onMouseMotion(x, y, relX, relY);
}

}
