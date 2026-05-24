drop table if exists table_1;
CREATE TABLE IF NOT EXISTS table_1 (
       analyte_id INT PRIMARY KEY -- file id
       , tR  REAL     -- sample name
       , loss      TEXT
       , Qformula TEXT
       , Qmass    REAL
       , FOREIGN KEY (analyte_id) REFERENCES analyte
       );

INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id,  78.0,   '-H','C2H3O', 43.01784113586057 FROM analyte WHERE canonical_smiles = 'CC(C)=O'; -- Acetone
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id,  80.0, '-CH3','C2H5O', 41.03857658044057 FROM analyte WHERE canonical_smiles = 'CC(C)O';  -- IPA
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id,  82.0,     '','C2H3N', 41.02600052110057 FROM analyte WHERE canonical_smiles = 'CC#N';    -- Acetonitrile
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id,  73.0,'-CH3O','C2H3O', 43.01784113586057 FROM analyte WHERE canonical_smiles = 'COC(C)=O'; -- Methyl acetate
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id,  85.0,  '-Cl','CH2Cl', 48.983954164230575 FROM analyte WHERE canonical_smiles = 'ClCCl';  -- Dichloromethane
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id,  94.0,'-C2H5','C4H9',  57.06987670872057  FROM analyte WHERE canonical_smiles = 'CCCCCC'; -- Hexane
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id,  98.0,'-C2H5','CH3O',  31.017841135860568 FROM analyte WHERE canonical_smiles = 'CCCO';   -- 1-Propanol
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 105.0,'-C2H5','C2H3O', 43.01784113586057  FROM analyte WHERE canonical_smiles = 'CCOC(C)=O'; --Ethyl acetate
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 109.0,'-CH2O','C3H6',  42.04640161251057  FROM analyte WHERE canonical_smiles = 'C1CCOC1';   -- THF
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 109.0,  '-Cl','CHCl2', 82.94498181216056  FROM analyte WHERE canonical_smiles = 'ClC(Cl)Cl'; -- Chloroform
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 117.0,'-C2+H3','C4H9', 57.06987670872057  FROM analyte WHERE canonical_smiles = 'c1ccccc1';  -- Benzene
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 118.0,'-C2H12','C6H6', 78.04640161251056  FROM analyte WHERE canonical_smiles = 'CC(C)CC(C)(C)C'; -- 2,2,4-Trimethylpentane
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 121.0,'-C4H9','C3H7',  43.054226644580574  FROM analyte WHERE canonical_smiles = 'CCCCCCC'; -- Heptane
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 121.0,'-C3H7','C2H3O', 43.01784113586057 FROM analyte WHERE canonical_smiles = 'CC(=O)C(C)C'; -- 3-Methyl-2-butanone
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 125.0,'-H2O','C4H8',   56.062051676650576 FROM analyte WHERE canonical_smiles = 'CCCCO'; -- 1-Butanol
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 130.0,'-C2H5','C3H5O', 57.03349120000057 FROM analyte WHERE canonical_smiles = 'CCC(=O)CC'; -- 3-Pentanone
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 133.0,'-C2H4','C3H6O2',74.03623085163056 FROM analyte WHERE canonical_smiles = 'CCCC(=O)OC'; -- MBOBu
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 145.0,     '','C5H5N', 79.04165058524056 FROM analyte WHERE canonical_smiles = 'c1ccncc1';   -- Pyridine
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 146.0,   '-H','C7H7',  91.05422664458057 FROM analyte WHERE canonical_smiles = 'Cc1ccccc1';    -- Toluene
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 155.0,'-C4H8','C2H4O', 44.025666167930574 FROM analyte WHERE canonical_smiles = 'CCCCCC=O';     -- Hexanal
INSERT OR REPLACE INTO table_1 (analyte_id,tR,loss,Qformula,Qmass) SELECT analyte.id, 169.0, '-CH3','C7H7',  91.05422664458057 FROM analyte WHERE canonical_smiles = 'Cc1cccc(C)c1'; -- m-Xylene
