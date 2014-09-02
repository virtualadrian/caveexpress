#pragma once

#include "UINode.h"
#include "engine/client/ui/BitmapFont.h"

class UINodeButton: public UINode {
protected:
	std::string _title;
	int _titleAlign;
	BitmapFontPtr _font;
	Color _fontColor;
public:
	UINodeButton (IFrontend *frontend, const std::string& title = "");
	virtual ~UINodeButton ();

	void setTitleAlignment (int align);
	void setTitle (const std::string& title);
	const std::string& getTitle () const;
	void setFont (const BitmapFontPtr& font, const Color& color = colorBlack);

	virtual float getAutoWidth () const override;
	virtual float getAutoHeight () const override;
	virtual bool isActive () const override;
	virtual void render (int x, int y) const override;
};

inline void UINodeButton::setTitle (const std::string& title)
{
	_title = title;
	_id = title;
	if (!_title.empty())
		autoSize();
}

inline void UINodeButton::setFont (const BitmapFontPtr& font, const Color& color)
{
	_font = font;
	Vector4Set(color, _fontColor);
	if (!_title.empty())
		autoSize();
}

inline bool UINodeButton::isActive () const
{
	return isVisible();
}

inline const std::string& UINodeButton::getTitle () const
{
	return _title;
}
