-- DROP TABLE IF EXISTS dgroup;

CREATE TABLE IF NOT EXISTS dgroup (
id INTEGER PRIMARY KEY
,dname TEXT
,grpid INTEGER
,sampid INTEGER
,UNIQUE(sampid)
,FOREIGN KEY(sampid) REFERENCES QuanSample(id)
);

INSERT INTO dgroup (dname, grpid, sampid )
       SELECT replace(name,'.' || replace(name,rtrim(name,replace(name,'.','')),''),''),id,id FROM
       	      (SELECT replace(dataSource,rtrim(dataSource,replace(dataSource,'/','')),'') AS name, id FROM QuanSample);

UPDATE dgroup SET grpid = -1 WHERE cast( substr(dname,-3,3) as int ) < 3; -- outliers if runno < 3
