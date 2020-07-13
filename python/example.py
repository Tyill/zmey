
import zmClient as zm


obj = zm.ZMObj(zm.dbType.PostgreSQL, "host=localhost port=5432 password=123 dbname=zmeyDb connect_timeout=10")

allErr = obj.getInternErrors(0, 0, 10)

err = obj.getLastError()

err