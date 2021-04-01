library(stringr)
library(igraph)

# significance threshold for tests
threshold = 0.1

# read list of top-5s and get stats
df = read.csv("parsed.txt", header=F, stringsAsFactor=F)
books = sort(unique(unlist(df[,seq(1:5)])))
n = length(df[,1])
nbooks = length(books)

# build set of paired books i,j
# initialise adjacency list for graph and color list for edges
adjlist = NULL
ecolors = NULL
# loop through each directed pair of non-identical books
for(i in 1:nbooks) {
  for(j in 1:nbooks) {
    if(i != j) {
    
      # get counts of occurrences of j in sets with and without i
      i.set = which(subset(df == books[i]), arr.ind=T)[,1]
      j.in = length(which(subset(df[i.set,] == books[j]), arr.ind=T)[,1])
      j.out = length(which(subset(df[-i.set,] == books[j]), arr.ind=T)[,1])
      n.in = length(df[i.set,1])
      n.out = length(df[-i.set,1])

      # construct contingency table for fisher
      test.matrix = matrix(c(j.in, n.in, j.out, n.out), nrow=2)

      # do fisher's exact test
      ctest = fisher.test(test.matrix)

      # if we (uncorrected!) cross the threshold, record this edge and output stats
      if(ctest$p.value < threshold) {
        if(j.in/n.in > j.out/n.out) {
	  dirn = "more"
	} else {
	  dirn = "less"
	}
	cat(paste(c(books[i], ",", dirn, ",", books[j], ",", format(100*j.in/n.in, digits=2), "% (", j.in, "/", n.in, ") vs ", format(100*j.out/n.out, digits=2), "% (", j.out, "/", n.out, ")\n"), collapse=""))
	adjlist = c(adjlist, c(i, j))

        # label edge color by direction of interaction 
	ecolors = c(ecolors, ifelse(dirn == "more", rgb(0.6,0.6,1), rgb(1,0.6,0.6)))
      }
    }
  }
}

# this function is taken from https://stackoverflow.com/questions/16875547/using-igraph-how-to-force-curvature-when-arrows-point-in-opposite-directions
# it coerces igraph to plot two curved edges when A <-> B
autocurve.edges2 <-function (graph, start = 0.5)
{
    cm <- count.multiple(graph)
    mut <-is.mutual(graph)  #are connections mutual?
    el <- apply(get.edgelist(graph, names = FALSE), 1, paste,
        collapse = ":")
    ord <- order(el)
    res <- numeric(length(ord))
    p <- 1
    while (p <= length(res)) {
        m <- cm[ord[p]]
        mut.obs <-mut[ord[p]] #are the connections mutual for this point?
        idx <- p:(p + m - 1)
        if (m == 1 & mut.obs==FALSE) { #no mutual conn = no curve
            r <- 0
        }
        else {
            r <- seq(-start, start, length = m)
        }
        res[ord[idx]] <- r
        p <- p + m
    }
    res
}

# construct edgelist and graph from adjacency list
edgelist = matrix(adjlist, ncol=2,byrow=T)
g = graph_from_edgelist(edgelist)

# decorative features including offsetting vertex labels by an amount proportion to the length in lines of the label
V(g)$label = gsub(" ","\n",books)
V(g)$label.dist = .3+.3*str_count(V(g)$label, "\n")
E(g)$color = ecolors
par(lheight=0.6)

# prune out disconnected nodes
singletons = which(degree(g)==0)
gprune = delete.vertices(g, singletons)

# use above function for curved edges and plot
curves <-autocurve.edges2(gprune)
plot(gprune, vertex.size=0, edge.curved=curves, vertex.label.cex=0.75, edge.arrow.size=.75, vertex.label.degree=pi/2)


