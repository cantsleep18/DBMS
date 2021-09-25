SELECT hometown
FROM Trainer
GROUP BY hometown
HAVING count(id)= 
		(SELECT Max(cnt)
         FROM(	
           		SELECT count(id) as cnt 
				FROM Trainer
				GROUP BY hometown) a
		) ;