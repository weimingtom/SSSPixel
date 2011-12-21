function dumpPack(s)
	local block = 16
	local i = 1
	while true do
		local bytes = s:sub(i, i+15)
		for _, b in ipairs { string.byte(bytes, 1, -1) } do
			io.write(string. format('%02X ', b))
		end

		io.write(string.rep('   ', block - string.len(bytes)))
		io.write(' ', string.gsub(bytes, '%c', '.'), '\n')

		i = i + 16
		if i > #s then break end
	end
end

function dumpPackfile(v, f, s)
	local block = 16
	local i = 1
	f:write(string.format("unsigned char %s[] = {\n", v))
	while true do
		local bytes = s:sub(i, i+15)
		for _, b in ipairs { string.byte(bytes, 1, -1) } do
			f:write(string.format('0x%02X, ', b))
		end
		f:write('\n')

		i = i + 16
		if i > #s then break end
	end
	f:write("};\n")
end

local fin = io.open("head.bmp", "rb")
local fout = io.open("raw.h", "w")
fin:read(0x36)
local s = fin:read("*a")
-- dumpPack(s)
dumpPackfile("head_bmp", fout, s)
fin:close()
fout:close()
