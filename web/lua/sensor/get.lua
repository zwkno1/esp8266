local redis = require "resty.redis"
local cjson = require "cjson"
local red = redis:new()
cjson.encode_empty_table_as_object(false)


local reply = {}

function get_data()
    red:set_timeout(1000) -- 1 second
    local ok, err = red:connect("127.0.0.1", 6379)
    if not ok then
        reply.msg = "failed to connect redis: " .. err
        return
    end

    local args = ngx.req.get_uri_args()
    local date = args["date"] or os.date("%Y_%m_%d")
    local key = "temp_" .. date

    local res, err = red:lrange(key, 0, -1)

    if not res then
        reply.msg = "failed to lrange: " .. err
        return
    end

    -- json encode the temperature data
    local temp_data = {}

    for i, v in ipairs(res) do
        temp_data[i] = v
    end

    reply.data = temp_data
    reply.msg = "success"
end

get_data()

ngx.header.content_type = "application/json"
ngx.say(cjson.encode(reply))