WITH manual_peaks AS (
    SELECT
        chroid AS ref_chroid,
        name AS compound,
        tR AS ref_tR
    FROM cpeak
    WHERE substr(name,1,1) NOT GLOB '[0-9]'
),
xic_peaks AS (
    SELECT
        chromatogram.id AS chroid,
        CAST(substr(chromatogram.name, 5) AS REAL) AS mass,
        cpeak.tR,
        cpeak.area,
        cpeak.height,
        cpeak.width
    FROM chromatogram
    JOIN cpeak
      ON cpeak.chroid = chromatogram.id
    WHERE chromatogram.name LIKE 'm/z %'
),
assigned AS (
    SELECT
        x.*,
        m.ref_chroid,
        m.compound,
        m.ref_tR,
        ABS(x.tR - m.ref_tR) AS dtR,
        ROW_NUMBER() OVER (
            PARTITION BY x.chroid, x.tR
            ORDER BY ABS(x.tR - m.ref_tR)
        ) AS rn
    FROM xic_peaks x
    JOIN manual_peaks m
      ON ABS(x.tR - m.ref_tR) <= 1.0
)
SELECT
    compound,
    ref_tR,
    COUNT(*) AS n_masses,
    MIN(tR) AS min_tR,
    MAX(tR) AS max_tR,
    MAX(dtR) AS max_dtR,
    group_concat(printf('%.6f', mass), ', ') AS masses
FROM assigned
WHERE rn = 1
GROUP BY ref_chroid, compound, ref_tR
ORDER BY ref_tR;
