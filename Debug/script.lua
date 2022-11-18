--require 'array'

function get_words(s)
	local words = {}
	for word in s:gmatch("%S+") do table.insert(words, word) end
	return words
end

function str_is_empty(s)
	return s == nil or s == ''
end

function split_str (inputstr, sep)
	if sep == nil then
			sep = "%s"
	end
	local t={}
	for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
			table.insert(t, str)
	end
	return t
end

-- see if the file exists
function file_exists(file)
	local f = io.open(file, "rb")
	if f then f:close() end
	return f ~= nil
end

-- get all lines from a file, returns an empty
-- list/table if the file does not exist
function lines_from(file)
	if not file_exists(file) then return {} end
		local lines = {}
		for line in io.lines(file) do
			lines[#lines + 1] = line
	end
	return lines
end


function send_micro_code(file)
	local lines = lines_from(file)

	local line_nums = #lines
	local i = 1
	local j = 0

	while (i <= line_nums) do
		local v = lines[i]

		--print('j: ', j)

		if not str_is_empty(v) then

			local line_end = string.find(v, ';')
			if line_end == nil then
				line_end = string.len(v) + 1
			end

			--print('arg1:', found, 'arg2:', line_end)

			local line = string.sub(v, 1, line_end - 1)

			local instruction = tonumber(line, 2)
			WriteUM(mia, j, instruction)
			print('Wrote instruction to memory:', line, '=', string.format("0x%.7x", instruction))

			j = j + 1
			--::continue::
		end
		i = i + 1
	end
end


function send_pm(file)
	local lines = lines_from(file)

	local line_nums = #lines
	local i = 1
	local j = 0
	local adr_offset = 0

	while (i <= line_nums) do
		local v = lines[i]

		if not str_is_empty(v) then
			local adr_offset_end = string.find(v, ':')

			if (adr_offset_end ~= nil) then
				adr_offset = tonumber(string.sub(v, 1, adr_offset_end - 1), 16) -- set address offset and continue to next iteration
				print(adr_offset)
				goto continue
			end

			local line_end = string.find(v, ';')
			if line_end == nil then
				line_end = string.len(v) + 1
			end

			local line = string.sub(v, 1, line_end - 1)
			local data = tonumber(line, 16)
			WritePM(mia, adr_offset, data)
			print('Wrote instruction to memory:', string.format("0x%.2x", adr_offset), '=', string.format("0x%.4x", data))

			adr_offset = adr_offset + 1

			::continue::
		end
		i = i + 1
	end
end


--collectgarbage("stop")
print('hello from lua!')
mia = initMia()
print(mia)

send_micro_code('C:\\Users\\cobea\\source\\repos\\Mia\\Debug\\microcode.umia')
send_pm('C:\\Users\\cobea\\source\\repos\\Mia\\Debug\\machine_code.mia')

while (true) do
	io.write('Enter command: ')

	local cmd = ReadUserInput()
	local words = get_words(cmd)

	if (words[1] == 'step') then
		print('Calling one ASM instruction')
		RunASMInstruction(mia)

	elseif (words[1] == 'stepu') then
		print('Calling one Micro instruction')
		RunMicroInstruction(mia)

	elseif (words[1] == 'read') then
		if (words[2] == 'um') then
			local mem = ReadAllUM(mia)
			for v, k in pairs(mem) do
				io.write(string.format('%.2x', v - 1) .. ': ' .. string.format('%.7x', k) .. ' ')
				if (v % 8 == 0) then print() end
			end
		elseif (words[2] == 'pm') then
			local mem = ReadAllPM(mia)
			for v, k in pairs(mem) do
				io.write(string.format('%.2x', v - 1) .. ': ' .. string.format('%.4x', k) .. ' ')
				if (v % 8 == 0) then print() end
			end
		end
	elseif (words[1] == 'exit') then
		break
	elseif (words[1] == 'break') then
		local stop = tonumber(words[2], 16)
		InsertBreakPoint(mia, stop)
		print(string.format('Inserted breakpoint at uPC=%.2x', stop))
	else
		print('Invalid command')
	end

	local _, pc, upc, supc, ir, asr, ar, hr, gr0, gr1, gr2, gr3, status_flags, lc, uIR, clock_cycles = GetMiaData(mia)
	pc = pc & 0xff
	upc = upc & 0xffff
	supc = supc & 0xffff
	ir = ir & 0xffff
	asr = asr & 0xff
	ar = ar & 0xffff
	hr = hr & 0xffff
	gr0 = gr0 & 0xffff
	gr1 = gr1 & 0xffff
	gr2 = gr2 & 0xffff
	gr3 = gr3 & 0xffff
	status_flags = status_flags & 0xffff
	lc = lc & 0xffff
	uIR = uIR & 0x1ffffff

	--print(string.format('data: %x, %x, %x, %x', pc, upc, supc, ir))
	print(string.format('MIA DATA:\nPC: %.2x\nuPC: %.4x\nsuPC: %.4x\nIR: %.4x\nASR: %.2x\nAR: %.4x\nHR: %.4x\nGR0: %.4x\nGR1: %.4x\nGR2: %.4x\nGR3: %.4x\nStatus flags: %.4x\nLC: %.4x\nuIR: %.7x\nClock cycles:%d',
		pc, upc, supc, ir, asr, ar, hr, gr0, gr1, gr2, gr3, status_flags, lc, uIR, clock_cycles))
	--print('MIA DATA:\nPC: ', pc, '\nuPC:', upc, '\nsuPC:', supc, '\nIR: ', ir, '\nASR:', asr, '\nAR: ', ar, '\nHR: ', hr, '\nGR0:', gr0, '\nGR1:', gr1, '\nGR2:', gr2, '\nGR3:', gr3, '\nStatus flags:', status_flags, '\nLC: ', lc, '\nuIR:', uIR)

	--local mia_data = runComputer(mia)
	--print(mia_data)
end
