initstars()
creobj2()

colangle = 0
sw = 1

camera = {
  x = 0,
  y = 0,
  z = 0,
  x2 = 0,
  y2 = 0,
  z2 = -1,
  anglex = 1,
  angley = 0,
  anglez = 0,
  zoom = 0.1
}

light = {
  x = 1,
  y = 3,
  z = 10
}

function _init()

end

function _update()


  if btnp(1)then 
    camera.zoom = camera.zoom - 0.05  
  end
  if btnp(2) then 
    camera.zoom = camera.zoom + 0.05  
  end

  

  if btnp(3) then 
    sw = 1
    camera.anglex  = camera.anglex - 2
  end

  if btnp(4) then   
    sw = 1
    camera.anglex  = camera.anglex + 2
  end


  if btnp(6) then 
    sw = 2
    camera.angley  = camera.angley - 2
  end
  if btnp(5) then   
    sw = 2
    camera.angley  = camera.angley + 2
  end
  

end 

function _draw()
  cls()

  rendr2( camera.anglex, camera.angley, sw, camera.zoom)
end