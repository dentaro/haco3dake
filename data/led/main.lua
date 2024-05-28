
x=64
y=60
cn = 1
ledf = 0
function setup()
  fillrect(20,20,8,8,7)
end

function _init()--1回だけ
    setup()
end

function input()

  if btn(1) >= 1 then x = x-3 end
  if btn(2) >= 1 then x = x+3 end
  if btn(3) >= 1 then 
    y = y-3
    if y<0 then y = 128 end
  end
  if btn(4) >= 1 then y = y+3 end

  if btn(3) >= 1 then 
    cn = cn + 1
  end
  if btnp(2) then 
    ledf = (ledf+1)%2
  end
end

function _update()--ループします
  input()
end
 
function _draw()--ループします


  led(0,1)
  wait(1000) --ピカーー

  led(0,0)
  wait(1000)

  led(0,1)
  wait(200) --ピカッ！

  led(0,0)
  wait(1000)
end








  