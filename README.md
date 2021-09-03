# fighting-fantasy-projects
Collection of projects related to Fighting Fantasy gamebooks

authors/
--------
Success probabilities of Fighting Fantasy gamebooks, plotted by author

Source data and R script to compute and visualise statistics of success probability in Fighting Fantasy gamebooks.

Needs R with `ggplot2` and `ggrepel` packages.

Source data is a summary of the excellent calculations and solutions by user "champskees" here https://fightingfantazine.proboards.com/board/30/fighting-fantasy-solutions.

wofm-solution/
--------------
`wofm.c` is a simulation of a putative optimal solution (and some others) of The Warlock of Firetop Mountain with minimum stats. This is a WIP and may contain bugs (it gives a different output to others!)

cots-solution/
--------------
`crypt.c` is a simulation of a putative optimal solution of Crypt of the Sorcerer. This is a WIP and may contain bugs (it gives a different output to others!)

top-5/
------
Analysis of which books appear together surprisingly often (or surprisingly rarely) in peoples' top 5 lists. Needs R with `igraph` and `stringr` packages.
