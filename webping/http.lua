-- http.lua

Tstart = tmr.now()

conn = nil
conn = net.createConnection(net.TCP, 0)

conn:on("receive", function(conn, payload)
  -- print(payload)
  local a, b = string.find( payload, "Hits:" )
  if a == nil then a = 0 end
  if b == nil then b = 0 end
  if b ~= 0 then b = b + 5 end
  print( string.sub(payload, a, b ))
end)

conn:on("connection", function(conn, payload)
  print("\nConnected")
  conn:send("GET /esp8266.php"
    .." HTTP/1.1\r\n"
    .."Host: mozgy.t-com.hr\r\n"
    .."Connection: close\r\n"
    .."Accept: */*\r\n"
    .."User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n"
    .."\r\n") 
end)

conn:on("disconnection", function(conn, payload)
  print("Disconnected")
end)

conn:connect(80, 'mozgy.t-com.hr')
