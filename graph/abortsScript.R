setwd("/home/douglas/STMArraySum/graph")

tg <- read.table("aborts.txt", sep="\t" ) # read a table from the file, using TAB as separator

library(ggplot2)
library(lattice)
library(plyr)
library(Rmisc) #needed for summarySE function
require(scales)

Legend <-"Configuration"
originalNames <- c("Default", "Worst", "ZBest")
newNames <- c("OS Default", "Worst Pin", "Best Pin")

# summarySE provides the standard deviation, standard error of the mean, and a (default 95%) confidence interval
tgc <- summarySE(tg, measurevar="V1", groupvars=c("V2","V3"))

#yticks <- c(0     211.701.172)
#Plot graphic
ggplot(tgc, aes(x=factor(V2), y=V1, fill=V3)) + 
  geom_bar(position=position_dodge(), stat="identity",
           colour="black", # Use black outlines,
           size=.3) +     # Thinner lines
  geom_errorbar(aes(ymin=V1-sd, ymax=V1+sd),
                size=.3,    # Thinner lines
                width=.2,
                position=position_dodge(.9)) +  
  ggtitle(expression(atop("STM array sum", atop(italic("16.000.000 elements"))))) +
  #ggtitle("LULESH") +
  xlab("Machines") +
  ylab("Total aborts") +
  annotate("text", x = 2.25, y = 250000000, label = "Less is better") + 
  #coord_cartesian(ylim = c(0, 300000000)) +
  scale_y_continuous(labels = comma) +
  #scale_y_continuous(limits=c(0,NA), breaks=yticks, labels=yticks) + 
  #scale_y_log10(breaks = trans_breaks("log10", function(x) 10^x),
  #                labels = trans_format("log10", math_format(10^.x))) +
  scale_fill_manual(values = c("#006699", "#993300", "#008833"),
                    name=Legend, # Legend label, use darker colors
                    breaks=originalNames,
                    labels=newNames) +
  theme(plot.title = element_text(hjust = 0.5)) #centered title
