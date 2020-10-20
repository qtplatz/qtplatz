-- DROP TABLE IF EXISTS dgroup;


SELECT dataSource,pkd,SGSpectrum.id FROM QuanSample,SGSpectrum WHERE QuanSample.id=idSample;

SELECT id,dataSource FROM QuanSample;
-- 1	/Users/toshi/data/z440/2020-10-09/SFE_Cholesterol_0005.adfs

SELECT * FROM SGSpectrum WHERE idSample=(SELECT id FROM QuanSample WHERE dataSource LIKE "%SFE_Cholesterol_0005.adfs")
-- id   idSample fcn   pkd      epochtime               injTime
-- -------------------------------------------------------------------------
-- 1	1	0	0	1602225638225908113	0.7419506600000001
-- 2	1	0	0	1602225638403616513	0.919718059
-- 3	1	0	0	1602225638581342208	1.0974264500000002

SELECT * FROM SGPeak
-- idSpectrum mass                       time                   intensity
-- -------------------------------------------------------------------------
-- 1	240.2145353888147		2.643500434074383e-05	0.043109397409036
-- 1	243.99540549884307		2.664118933548388e-05	0.11465650241874285

-- peak list for idSpectrum=1
SELECT idSample,id,pkd,epochTime,injTime,idSpectrum,mass,time,intensity FROM SGSpectrum,SGPeak WHERE idSpectrum=1
-- idSample id pkd       epochTime              injTime             idSpectrum   mass                            time                    intensity
-- -------------------------------------------------------------------------
--1	1	0	1602225638225908113	0.7419506600000001	1	240.2145353888147		2.643500434074383e-05	0.043109397409036
--1	1	0	1602225638225908113	0.7419506600000001	1	243.99540549884307		2.664118933548388e-05	0.11465650241874285
--1	1	0	1602225638225908113	0.7419506600000001	1	244.35417245931896		2.6660670993505203e-05	0.08407661246966906


SELECT id FROM SGSpectrum WHERE idSample=1
--id
---------
--1
--2
--3

WITH RECURSIVE r AS (
SELECT id FROM SGSpectrum WHERE idSample=1
UNION ALL
SELECT idSample,id,pkd,epochTime,injTime,idSpectrum,mass,time,intensity FROM SGPeak WHERE idSpectrum=r.id
) SELECT * FROM r


--
SELECT * FROM SGSpectrum,SGPeak WHERE id=idSpectrum AND idSample=1


---
WITH RECURSIVE r AS (
     SELECT * FROM SGSpectrum,SGPeak WHERE id=idSpectrum AND idSample=1
     UNION ALL
     SELECT SGSpectrum.*,SGPeak.* FROM SGSpectrum,SGPeak,r WHERE idSpectrum = r.id
) SELECT * FROM r


WITH RECURSIVE r AS (
     SELECT * FROM SGSpectrum,SGPeak WHERE id=idSpectrum AND idSample=1
) SELECT * FROM r

--------
SELECT * FROM SGSpectrum,SGPeak WHERE id=idSpectrum AND idSample=1 AND mass < 250
