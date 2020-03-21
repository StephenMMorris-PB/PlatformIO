CREATE USER powerAppsSqlUser WITH PASSWORD = 'U87&ksh>>b4Dp'
CREATE ROLE powerAppsUsers
EXEC sp_addrolemember N'db_datareader', N'powerAppsUsers'
EXEC sp_addrolemember N'db_datawriter', N'powerAppsUsers'
ALTER ROLE powerAppsUsers ADD MEMBER powerAppsSqlUser;