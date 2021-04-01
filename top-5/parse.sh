tr [:upper:] [:lower:] < survey.txt | sed 's/[.]//g' | sed 's/é/e/g' | sed 's/- //g' | sed 's/://g' | sed 's/khare,/khare cityport of traps,/g' | sed 's/phantasms/phantoms/g' | sed 's/the //g' | sed 's/city thieves/city of thieves/g' | sed 's/rouge/rogue/g' | sed 's/plant/planet/g' | sed 's/deathrap/deathtrap/g' > parsed.txt
 
