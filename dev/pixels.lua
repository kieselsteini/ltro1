local data = {}
for i = 1, 256 do
    data[i] = '0'
end
for ch in string.gmatch('0123456789', '.') do
    data[string.byte(ch) + 1] = string.format('%d', tonumber(ch))
end
for i = 1, 256, 16 do
    print('    ' .. table.concat(data, ', ', i, i + 15) .. ',')
end
