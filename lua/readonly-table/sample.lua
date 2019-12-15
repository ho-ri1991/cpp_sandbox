print(readonly_table.ID)
print(getmetatable(readonly_table))
print(#readonly_table)
print(#readonly_table.Image)
print(#readonly_table.Image.IDs)
--setmetatable(readonly_table, {}) --error
--readonly_table.ID = 1 --error
--readonly_table.ID1 = 1 --error
--readonly_table = {} --error
--rawset(_G, "readonly_table", {}) --error

x = readonly_table.Image.Width

return 0
