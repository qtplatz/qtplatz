SELECT *,(
        SELECT synonym.name
        FROM synonym
        WHERE synonym.analyte_id = analyte.id
        ORDER BY synonym.rowid
        LIMIT 1
    ) AS synonym FROM analyte,product WHERE analyte.id = product.analyte_id
