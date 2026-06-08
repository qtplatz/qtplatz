WITH params AS (
    SELECT
        0.5 AS bin_width,
        0.5 AS bin_offset
),
xic_peaks AS (
    SELECT
        chromatogram.id AS chroid,
        CAST(substr(chromatogram.name, 5) AS REAL) AS mass,
        cpeak.*,
        CAST((cpeak.tR + params.bin_offset) / params.bin_width AS INTEGER) AS tr_bin
    FROM chromatogram
    JOIN cpeak
      ON cpeak.chroid = chromatogram.id
    CROSS JOIN params
    WHERE chromatogram.name LIKE 'm/z %'
)
SELECT
    MIN(tR) AS tR_min,
    MAX(tR) AS tR_max,
    AVG(tR) AS avg_tR,
    COUNT(*) AS n_masses,
    group_concat(printf('%.6f', mass), ', ') AS masses
FROM xic_peaks
GROUP BY tr_bin
HAVING COUNT(*) > 1
ORDER BY avg_tR;
