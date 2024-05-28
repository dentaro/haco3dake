
x=64
y=60
cn = 1
num = 0
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
    num = (num+1)%4
  end
  
end

function _update()--ループします
  input()
end
 

function _draw()--ループします
  -- rectfill(56, 32, 48, 90, 0, "/anime/png/enemy/3.png") --160,48が敵エリアの最大値
  -- rectfill(56, 32, 48, 90, 0, "/anime/png/spr" .. tostring(num) ..".png") --160,48が敵エリアの最大値
  -- cls(0)
  color(3)
  -- fillcircle(64,64,64)
  
  if num == 0 then
    rectfill(0,0, 159, 127, 0, "/anime/png/spr0.png") --160,128はが敵エリアの最大値
    -- png("/anime/png/spr0.png",0,0) --160,48が敵エリアの最大値
  elseif num == 1 then
    rectfill(0,0, 159, 127, 0, "/anime/png/spr1.png") 
  elseif num == 2 then
    rectfill(0,0, 47, 47, 0, "/anime/png/3.png") --48,48が敵エリアの最大値
  elseif num == 3 then
    rectfill(0,0, 159, 127, 0, "/anime/png/spr1.png") --160,48が敵エリアの最大値
  end  
end








  