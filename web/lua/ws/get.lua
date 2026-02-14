local websocket = require "resty.websocket.server"
local redis = require "resty.redis"
local cjson = require "cjson"

local function handle_connection()
    ngx.log(ngx.INFO, "WebSocket connection established")
    local ws, err = websocket:new {
        timeout = 60000,
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
    ngx.log(ngx.ERR, "Received data: ", data, ", len: ", #data, ", type: ", typ)

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

    while true do
        local res, err = red:lrange(key, offset, -1)
        if err then
            return ngx.exit(444)
        end

        local data = ""
        local count = 0

        local function send_data()
            if #data > 0 then
                local ok, err = ws:send_text(data)
                if not ok then
                    return ngx.exit(444)
                end
                data = ""
                count = 0
            end
        end

        for i, v in ipairs(res) do
            offset = offset + 1
            count = count + 1

            if #data > 0 then
                data = data .. ";" .. v
            else
                data = v
            end

            if count >= 360 then
                send_data()
            end
        end
        send_data()

        ngx.sleep(1)
    end
end

handle_connection()
