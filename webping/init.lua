--init.lua

tmr.alarm(3, 300000, 1, function() dofile('init.lua') end)
 
wifi.sta.config("MozzWiFi","xx")
if wifi.sta.getip() ~= nil then
  wifi.sta.connect()
end

tmr.alarm(1, 5000, 1, function()
  if wifi.sta.getip() == nil then
    print("IP unavailable, Waiting...")
  else
    tmr.stop(1)
    print("ESP8266 mode is: " .. wifi.getmode())
    print("The module MAC address is: " .. wifi.ap.getmac())
    print("Config done, IP is "..wifi.sta.getip())
    dofile ("http.lua")
  end
end)

print("Heap: " .. node.heap())

