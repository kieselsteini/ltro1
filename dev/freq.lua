local data = {}
for i = 1, 88 do
    data[i] = string.format('%8.3ff', 440.0 * 2.0 ^ ((i - 49) / 12))
end
for i = 1, #data, 8 do
    print(table.concat(data, ', ', i, i + 7) .. ',')
end
