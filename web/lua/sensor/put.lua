
-- get arguments from the request
local args = ngx.req.get_uri_args()
local temp = args["data"]
if not temp then
    ngx.say("missing temp argument")
    return
end

--[[
-- validate the temp argument
local temp_num = tonumber(temp)
if not temp_num then
    ngx.say("invalid temp argument")
    return
end

-- validate the temp argument is within a reasonable range
if temp_num < -500 or temp_num > 1000 then
    ngx.say("temp argument out of range")
    return
end
]]--

local redis = require "resty.redis"
local red = redis:new()
red:set_timeout(1000) -- 1 second

local ok, err = red:connect("127.0.0.1", 6379)
if not ok then
    ngx.say("failed to connect: ", err)
    return
end

key = os.date("temp_%Y_%m_%d")

-- caculate the number of minutes since 00:00
local minutes = os.date("%H") * 60 + os.date("%M")
-- store the temp value in a list with the key "temp_YYYY_MM_DD" and the value "minutes:temp"
local res, err = red:rpush(key, minutes .. ":" .. temp)

if not res then
    ngx.say("failed to rpush: ", err)
    return
end

red:expire(key, 24 * 60 * 60 * 7) -- set the key to expire in 1 week

ngx.say("temp recorded: ", temp)