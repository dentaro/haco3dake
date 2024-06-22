--  launcher
-- count = 0
-- color(12)
count = 0
cursor = 0
scroll = 0
tabCursor = 0
showmax = 8 --ファイルの最大表示数0~
lineheight = 13

-- function getid()
--   -- if tp(1)
-- end


function setup()

end

function _init()
  -- fs = list() 
  fnum = list(-1)--　　-1でファイルの数を取得
end

function drawTab(x, y, w)
  for i = 1,w do
    spr8(50, x + i*8,y-4)
    spr8(50, x + i*8,y+4)
  end
end


function drawDisableTab(x, y, w)
  spr(x, y,8,8,32,8,8,8) -- left
  for i = 1,w do
    spr(x + i*8, y,8,8,40,8,8,8) -- middle
  end
  spr(x + w*8 ,y,8,8,48,8,8,8) -- right
end

-- function drawTile(x, y, w, h)
--   for i=0,h-1 do
--     for j=0,w-1 do
--       spr(x + j*8, y + i*8,8,8,40,24,8,8)
--     end
--   end
-- end

function drawFile()
  drawTab(0,10,4)
  color(7)--textcol
  text("file", 12, 1)

  drawDisableTab(44,1,4)
  color(1)--textcol
  text("util", 52, 0)

  fillrect(0, (cursor-scroll)*lineheight+13, 160, 13, 8)
  color(7)--textcol

  for i = scroll, fnum do
    if i - scroll >= 0 and i - scroll <= showmax then
      v = list(i)--fs[i] --0以上でパス名を取得
      -- text(i .. v, 10, 20 + lineheight * (i-scroll))--先頭数字
      text(v, 10, 13 + lineheight * (i-scroll))--先頭数字なし
    end
  end

  -- spr(0, 4 + (cursor-scroll)*lineheight, 8, 8, 32, 16, 8, 8)
  
  spr8(21, 0, 4 + (cursor-scroll)*lineheight+13)


  if btn(3) >= 1 then
    cursor = cursor - 1
    if cursor < 0 then
      cursor = fnum
    end
  end

  if btn(4) >= 1 then
    cursor = cursor + 1
    if cursor > fnum then
      cursor = 0
    end
  end

  if cursor < scroll then
    scroll = cursor
  end
  if cursor > showmax then
    scroll = cursor - showmax
  end

  -- if btn(1) == 2 then
  --   run(list(cursor))
  -- end

  if btn(5) >= 1 then
    appmode(list(cursor),1)--エディットモードで再起動
  end

  if btn(6) >= 1 then
    -- appmode(list(cursor),0)--アプリモードで再起動（エディタに戻るときはこっち）
    run(list(cursor))--再起動せずにアプリを起動
  end
end

-- utilMenu = {"reload", "wifi on", "self wifi on", "reboot", "tone on", "tone off"}

-- function drawUtil()
--   drawDisableTab(0,10,4)
--   color(1)
--   text("file", 12, 9)

--   drawTab(40,10,4)
--   color(1)
--   text("util", 52, 10)

--   for k, v in pairs(utilMenu) do
--     text((k - 1) .. ":" .. v, 10, 20 + 10 * (k - 1))
--   end
--   --fillrect(0, 20 + cursor * 10, 10, 10)
--   spr(0, 20 + cursor*10, 8, 8, 32, 16, 8, 8)

--   if btn(3) == 2 then
--     cursor = cursor - 1
--     if cursor < 0 then
--       cursor = #utilMenu - 1
--     end
--   end

--   if btn(4) == 2 then
--     cursor = cursor + 1
--     if cursor >= #utilMenu then
--       cursor = 0
--     end
--   end

--   if btn(1) == 2 then
--     if cursor == 0 then
--       run("/init/main.lua")
--     elseif cursor == 1 then
--      --ATPモード：共有のWifiに入るモード（通常はこちら/init/param/wifipass.txtを書き換えることで設定できる）
--       if not(iswifidebug()) then
--         wifiserve()
--       end
--     elseif cursor == 2 then
--      --APモード：ESP32自体がアクセスポイントになるモード
--       if not(iswifidebug()) then
--         wifiserve("ap")
--       end
--     elseif cursor == 3 then
--       reboot()
--     elseif cursor == 4 then
--      --  tone(0, 523)
--      --  tone(1, 659)
--       -- tone(2, 784)
--     elseif cursor == 5 then
--       -- tone(0, 0)
--       -- tone(1, 0)
--       -- tone(2, 0)
--     end
--   end
-- end

function _update()
  cls(1)--bgcol
  color(1)
  fillrect(0, 0, 160, 13)
  -- color(7)
  -- if iswifidebug() then
  --   text("wifi: on", 0, 0)
  --   text(getip(), 50, 0)
  -- else
  --   text("wifi: off", 0, 0)
  -- end

  -- if btn(0) == 2 then
  --   tabCursor = tabCursor - 1
  --   if tabCursor < 0 then
  --     tabCursor = 1
  --   end
  --   cursor = 0
  -- end
  -- if btn(2) == 2 then
  --   tabCursor = tabCursor + 1
  --   if tabCursor > 1 then
  --     tabCursor = 0
  --   end
  --   cursor = 0
  -- end

  -- drawTile(0,18,16,12)

  if tabCursor == 0 then
    drawFile()
  else
    -- drawUtil()
  end
  count = count + 1
  spr8(14,tp(0),tp(1))
end