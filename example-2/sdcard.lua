spi.setup(1, spi.MASTER, spi.CPOL_LOW, spi.CPHA_LOW, 8, 8)
print(spi)
vol = file.mount("/SD0", 8)
file.open("/SD0/test.txt")
print(file.read())
file.close()

-- spi.setup(1, spi.MASTER, spi.CPOL_LOW, spi.CPHA_LOW, 8, 8)
-- vol = file.mount("/SD0", 8)
-- file.chdir("/SD0")
-- l = file.list()
-- for k, v in pairs(l) do
--   print("name:" .. k .. ", size:" .. v)
-- end
