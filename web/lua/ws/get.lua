local websocket = require "resty.websocket.server"
local redis = require "resty.redis"
local cjson = require "cjson"

local function push(ws, data)
    local msg = ""
    local count = 0

    local function send_data()
        if #msg > 0 then
            local ok, err = ws:send_text(msg)
            if not ok then
                return ngx.exit(444)
            end
            msg = ""
            count = 0
        end
    end

    for i, v in ipairs(data) do
        count = count + 1

        if #msg > 0 then
            msg = msg .. ";" .. v
        else
            msg = v
        end

        if count >= 360 then
            send_data()
        end
    end
    send_data()
end

local function handle_connection()
    ngx.log(ngx.INFO, "WebSocket connection established")
    local ws, err = websocket:new {
        timeout = 30000,
        max_payload_len = 65535
    }

    if not ws then
        ngx.log(ngx.ERR, "create websocket: ", err)
        return ngx.exit(444)
    end

    ngx.log(ngx.INFO, "WebSocket connection ready: ", tostring(ws))

    local data, typ, err = ws:recv_frame()
    if typ ~= "text" then
        return ngx.exit(444)
    end

    if not data then
        return ngx.exit(444)
    end

    data = cjson.decode(data)
    if not data.date then
        return ngx.exit(444)
    end

    local red = redis:new()
    red:set_timeout(1000) -- 1 second
    local ok, err = red:connect("127.0.0.1", 6379)
    if not ok then
        return ngx.exit(444)
    end
    ngx.log(ngx.INFO, "Connected to Redis")

    local key = "temp_" .. data["date"]
    local offset = tonumber(data["offset"]) or 0

    local idleCount = 0

    while true do
        local res, err = red:lrange(key, offset, -1)
        if err then
            return ngx.exit(444)
        end

        if #res ~= 0 then
            push(ws, res)
            offset = offset + #res
            idleCount = 0
        else
            idleCount = idleCount + 1
        end

        if idleCount >= 10 then
            ws:send_ping()
            idleCount = 0
        end

        ngx.sleep(3)
    end
end

handle_connection()
