# R script to visualise statistics of success probability in Fighting Fantasy gamebooks, by author
# source CSV file contains the calculations by "champskees" here https://fightingfantazine.proboards.com/board/30/fighting-fantasy-solutions
# needs ggplot2 and ggrepel packages

## install packages -- uncomment if necessary
# install.packages("devtools")
# devtools::install_github("slowkow/ggrepel")
# install.packages("ggplot2")

# invoke packages
library(ggplot2)
library(ggrepel)

# read data
df = read.csv("difficulty.csv")

# construct list of authors and populate lists of (product of success probabilities using top stats) and (product of success probabilities using top stats) where products are taken over all of an author's books
authors = unique(df$Author)
top.products = bottom.products = NULL
for(i in 1:length(authors)) {
  top.products = c(top.products, prod(df$Top[df$Author == authors[i]]/100))
  bottom.products = c(bottom.products, prod(df$Bottom[df$Author == authors[i]]/100))
}

# make a new dataframe with those products
product.df = data.frame(authors, top.products, bottom.products)

# first plot is just the product probabilities we just computed
plot.product.probability <- ggplot(product.df, aes(x=reorder(authors,top.products, FUN=median), y=top.products*100)) + geom_boxplot() + theme(axis.text.x = element_text(angle = 90, vjust = 0.5, hjust=1)) + labs(x="Author(s)", y = "Probability of success through\nall books as % (with top stats)") + theme(axis.text.x = element_text(size=14), axis.text.y = element_text(size=14), axis.title = element_text(size=16))

# second plot is the individual book stats. these are plotted both as bar plots and individual datapoints, with labels giving the number of each book (using ggrepel for clarity)
plot.individual.probability <- ggplot(df, aes(x=reorder(Author,Top, FUN=median), y=Top)) + geom_boxplot(outlier.shape=NA) + theme(axis.text.x = element_text(angle = 90, vjust = 0.5, hjust=1)) + labs(x="Author(s)", y = "Individual book success probabilities\nas % (with top stats)") + geom_point() + geom_text_repel(aes(label=Number), color="gray", max.overlaps=20) + theme(axis.text.x = element_text(size=14), axis.text.y = element_text(size=14), axis.title = element_text(size=16))

# third plot is this restyled for James! :-)
plot.individual.probability.james <- ggplot(df, aes(x=reorder(Author,Top, FUN=mean), y=Top)) + geom_boxplot(outlier.shape=NA) + theme(axis.text.x = element_text(angle = 90, vjust = 0.5, hjust=1)) + labs(x="Author(s)", y = "Individual book success probabilities\nas % (with top stats)") + geom_point() + geom_text_repel(aes(label=Number), color="gray", max.overlaps=20) + theme(axis.text.x = element_text(size=14), axis.text.y = element_text(size=14), axis.title = element_text(size=16)) + coord_flip()

# store these plots to files
pdf(file="plot-product-probability.pdf")
plot.product.probability
dev.off()
pdf(file="plot-individual-probability.pdf", width=10)
plot.individual.probability
dev.off()
pdf(file="plot-individual-probability-james.pdf")
plot.individual.probability.james
dev.off()
