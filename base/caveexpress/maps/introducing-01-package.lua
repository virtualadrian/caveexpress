function getName()
	return "Package"
end

function onMapLoaded()
end

function initMap()
	-- get the current map context
	local map = Map.get()
	map:addTile("tile-background-03", 0, 0)
	map:addTile("tile-background-04", 0, 1)
	map:addTile("tile-ground-04", 0, 2)
	map:addTile("tile-rock-big-01", 0, 3)
	map:addTile("tile-rock-03", 0, 5)
	map:addTile("tile-rock-big-01", 0, 6)
	map:addTile("tile-background-03", 1, 0)
	map:addTile("tile-background-03", 1, 1)
	map:addTile("tile-ground-04", 1, 2)
	map:addTile("tile-rock-02", 1, 5)
	map:addTile("tile-background-big-01", 2, 0)
	map:addTile("tile-background-big-01", 2, 2)
	map:addTile("tile-background-03", 2, 4)
	map:addTile("tile-packagetarget-rock-01-idle", 2, 5)
	map:addTile("tile-rock-03", 2, 6)
	map:addTile("tile-rock-02", 2, 7)
	map:addTile("tile-background-02", 3, 4)
	map:addTile("tile-background-02", 3, 5)
	map:addTile("bridge-wall-left-01", 3, 5)
	map:addTile("tile-background-big-01", 3, 6)
	map:addTile("tile-background-cave-art-01", 4, 0)
	map:addTile("tile-background-02", 4, 1)
	map:addTile("tile-background-big-01", 4, 2)
	map:addTile("tile-background-02", 4, 4)
	map:addTile("tile-background-02", 4, 5)
	map:addTile("bridge-plank-01", 4, 5)
	map:addTile("tile-background-03", 5, 0)
	map:addTile("tile-background-02", 5, 1)
	map:addTile("tile-background-02", 5, 4)
	map:addTile("tile-background-02", 5, 5)
	map:addTile("bridge-wall-right-01", 5, 5)
	map:addTile("tile-background-02", 5, 6)
	map:addTile("tile-background-01", 5, 7)
	map:addTile("tile-background-02", 6, 0)
	map:addTile("tile-background-big-01", 6, 1)
	map:addTile("tile-background-02", 6, 3)
	map:addTile("tile-background-03", 6, 4)
	map:addTile("tile-ground-03", 6, 5)
	map:addTile("tile-rock-big-01", 6, 6)
	map:addTile("tile-background-02", 7, 0)
	map:addTile("tile-background-01", 7, 3)
	map:addTile("tile-background-cave-art-01", 7, 4)
	map:addTile("tile-ground-01", 7, 5)
	map:addTile("tile-background-01", 8, 0)
	map:addTile("tile-background-03", 8, 1)
	map:addTile("tile-ground-04", 8, 2)
	map:addTile("tile-rock-02", 8, 3)
	map:addTile("tile-rock-big-01", 8, 4)
	map:addTile("tile-rock-02", 8, 6)
	map:addTile("tile-rock-03", 8, 7)
	map:addTile("tile-background-02", 9, 0)
	map:addTile("tile-background-04", 9, 1)
	map:addTile("tile-ground-03", 9, 2)
	map:addTile("tile-rock-03", 9, 3)
	map:addTile("tile-rock-02", 9, 6)
	map:addTile("tile-rock-02", 9, 7)
	map:addTile("tile-rock-02", 10, 0)
	map:addTile("tile-rock-big-01", 10, 1)
	map:addTile("tile-rock-02", 10, 3)
	map:addTile("tile-rock-02", 10, 4)
	map:addTile("tile-rock-03", 10, 5)
	map:addTile("tile-rock-03", 10, 6)
	map:addTile("tile-rock-02", 10, 7)
	map:addTile("tile-rock-03", 11, 0)
	map:addTile("tile-rock-03", 11, 3)
	map:addTile("tile-rock-03", 11, 4)
	map:addTile("tile-rock-02", 11, 5)
	map:addTile("tile-rock-02", 11, 6)
	map:addTile("tile-rock-02", 11, 7)

	map:addEmitter("item-package", 2.2, 0, 1, 200, "")
	map:addEmitter("item-package", 6.2, 4, 1, 0, "")

	map:setSetting("width", "12")
	map:setSetting("height", "8")
	map:setSetting("fishnpc", "false")
	map:setSetting("flyingnpc", "false")
	map:setSetting("gravity", "9.81")
	map:setSetting("introwindow", "intro1")
	map:setSetting("packagetransfercount", "2")
	map:setSetting("playerX", "6")
	map:setSetting("playerY", "3")
	map:setSetting("points", "100")
	map:setSetting("referencetime", "10")
	map:setSetting("sideborderfail", "false")
	map:setSetting("theme", "rock")
	map:setSetting("tutorial", "true")
	map:setSetting("waterchangespeed", "0")
	map:setSetting("waterfallingdelay", "0")
	map:setSetting("waterheight", "0.8")
	map:setSetting("waterrisingdelay", "0")
	map:setSetting("wind", "0")
end
