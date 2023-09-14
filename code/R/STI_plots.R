library(tidyverse)
library(scales)
library(dygraphs)
library(xts)
library(respR)
library(lubridate)


# load the data for different parameters
# CSI data
files <- list.files(path = "../../data/sleep_lab/clean_data/esp/right", pattern = "*.CSV", full.names = T)
full_data <- data_frame()
for (file in files) {
  overflow_index <- 57214
  data <- read_csv(file)
  data$local_timestamp[overflow_index:nrow(data)] <- data$local_timestamp[overflow_index:nrow(data)] + 4294967295
  
  file_type <- tools::file_path_sans_ext(basename(file))
  
  data <- data %>% mutate(pose = "none", condition = "none", filename = file_type)
  # assign the conditions manually
  # order of poses: back, left, stomach, right
  # order of conditions: without blanket, with blanket
  data$pose[14927:19242] = "back"
  data$condition[14927:19242] = "open"
  
  data$pose[25470:28543] = "left"
  data$condition[25401:28543] = "open"
  
  data$pose[34215:36099] = "stomach"
  data$condition[34215:36099] = "open"
  
  data$pose[39726:44131] = "right"
  data$condition[39726:44131] = "open"
  
  
  data$pose[47284:51867] = "back"
  data$condition[47284:51867] = "blanket"
  
  data$pose[55271:57690] = "left"
  data$condition[55271:57690] = "blanket"
  
  data$pose[60798:61955] = "stomach"
  data$condition[60798:61955] = "blanket"
  
  data$pose[64565:69201] = "right"
  data$condition[64565:69201] = "blanket"
  
  full_data <- rbind(full_data, data)
}

plot <- full_data  %>% filter(filename=="CSI") %>% mutate(time=local_timestamp/1000000) %>% select(time, sti) 
p <- dygraph(plot, xlab = "time (in seconds)", ylab = "STI") %>%
  dyOptions(drawPoints = TRUE, pointSize = 2) %>%
  dyRangeSelector()

show(p)

# Define the main directory path
main_directory <- "../../data/sleep_lab/clean_data/esp"

# List all subdirectories (conditions) in the main directory
device_dirs <- list.dirs(main_directory, full.names = FALSE, recursive = FALSE)

# Initialize an data frame
device_data <- data_frame()

reference_data <- full_data %>% filter(filename=="CSI")

# Loop through condition directories
for (device_dir in device_dirs) {
  file <- file.path(main_directory, device_dir, "CSI.CSV")
  data <- read_csv(file)
  
  # deal with timestamp overflows
  indices <- which(data$local_timestamp[2:nrow(data)] < data$local_timestamp[1:(nrow(data)-1)])
  print(device_dir)
  print(indices)
  for(i in indices){
    data$local_timestamp[(i+1):nrow(data)] <- data$local_timestamp[(i+1):nrow(data)] + 4294967295
  }
  
  
  
  start_index <- c(14927, 25470, 34215, 39726, 47284, 55271, 60798, 64565)
  stop_index <- c(19242, 28543, 36099, 44131, 51867, 57690, 61955, 69201)
  pose <- c("back", "left", "stomach", "right", "back", "left", "stomach", "right")
  condition <- c("open", "open", "open", "open", "blanket", "blanket", "blanket", "blanket")
  
  
  
  # Specify the output file name with the .svg extension
  # output_file <- "time_series_plot.pdf"
  # library(webshot)
  # # Export the dygraph plot as an SVG image
  # htmlwidgets::saveWidget(graph, file = output_file)
  file_type <- tools::file_path_sans_ext(basename(file))
  data <- data %>% mutate(pose = "none", condition = "none", device = device_dir)
  # assign the conditions manually
  # order of poses: back, left, stomach, right
  # order of conditions: without blanket, with blanket
  
  device_data <- rbind(device_data, data)
}


start_index <- c(14927, 25470, 34215, 39726, 47284, 55271, 60798, 64565)
stop_index <- c(19242, 28543, 36099, 44131, 51867, 57690, 61955, 69201)
pose <- c("back", "left", "stomach", "right", "back", "left", "stomach", "right")
condition <- c("open", "open", "open", "open", "blanket", "blanket", "blanket", "blanket")

data <- device_data %>% filter(device == "left")

start_is <- c()
stop_is <- c()
for(i in seq(8)){
  start_t <- reference_data$local_timestamp[start_index[i]]
  stop_t <- reference_data$local_timestamp[stop_index[i]]
  start_i <- which(data$local_timestamp > start_t)[1]-1
  stop_i <- which(data$local_timestamp > stop_t)[1]-1
  print(start_i)
  start_is <- c(start_is, start_i)
  stop_is <- c(stop_is, stop_i)
  print(stop_i)
  data$pose[start_i:stop_i] = pose[i]
  data$condition[start_i:stop_i] = condition[i]
}
start_is[1]
plot$time[start_is[1]]

h = 200
w = 2100

plot <- data  %>% mutate(time=local_timestamp/1000000) %>% select(time, sti)
p1 <- dygraph(plot, main = "left", xlab = "time (in seconds)", ylab = "STI", group = "sti_plots", width = w, height = h) %>%
  dyOptions(drawPoints = F, pointSize = 2) %>% 
  dyShading(from = plot$time[start_is[1]], to = plot$time[stop_is[1]], color = "#F0FFFF") %>% 
  dyShading(from = plot$time[start_is[2]], to = plot$time[stop_is[2]], color = "#FFE1FF") %>% 
  dyShading(from = plot$time[start_is[3]], to = plot$time[stop_is[3]], color = "#C1FFC1") %>% 
  dyShading(from = plot$time[start_is[4]], to = plot$time[stop_is[4]], color = "#FFF8DC") %>% 
  dyShading(from = plot$time[start_is[5]], to = plot$time[stop_is[5]], color = "#BFEFFF") %>% 
  dyShading(from = plot$time[start_is[6]], to = plot$time[stop_is[6]], color = "#FFBBFF") %>% 
  dyShading(from = plot$time[start_is[7]], to = plot$time[stop_is[7]], color = "#9AFF9A") %>% 
  dyShading(from = plot$time[start_is[8]], to = plot$time[stop_is[8]], color = "#FFE4C4")

data <- device_data %>% filter(device == "middle")

start_is <- c()
stop_is <- c()
for(i in seq(8)){
  start_t <- reference_data$local_timestamp[start_index[i]]
  stop_t <- reference_data$local_timestamp[stop_index[i]]
  start_i <- which(data$local_timestamp > start_t)[1]-1
  stop_i <- which(data$local_timestamp > stop_t)[1]-1
  print(start_i)
  start_is <- c(start_is, start_i)
  stop_is <- c(stop_is, stop_i)
  print(stop_i)
  data$pose[start_i:stop_i] = pose[i]
  data$condition[start_i:stop_i] = condition[i]
}
start_is[1]
plot$time[start_is[1]]

plot <- data  %>% mutate(time=local_timestamp/1000000) %>% select(time, sti)
p2 <- dygraph(plot, main = "middle", xlab = "time (in seconds)", ylab = "STI", group = "sti_plots", width = w, height = h) %>%
  dyOptions(drawPoints = F, pointSize = 2) %>% 
  dyShading(from = plot$time[start_is[1]], to = plot$time[stop_is[1]], color = "#F0FFFF") %>% 
  dyShading(from = plot$time[start_is[2]], to = plot$time[stop_is[2]], color = "#FFE1FF") %>% 
  dyShading(from = plot$time[start_is[3]], to = plot$time[stop_is[3]], color = "#C1FFC1") %>% 
  dyShading(from = plot$time[start_is[4]], to = plot$time[stop_is[4]], color = "#FFF8DC") %>% 
  dyShading(from = plot$time[start_is[5]], to = plot$time[stop_is[5]], color = "#BFEFFF") %>% 
  dyShading(from = plot$time[start_is[6]], to = plot$time[stop_is[6]], color = "#FFBBFF") %>% 
  dyShading(from = plot$time[start_is[7]], to = plot$time[stop_is[7]], color = "#9AFF9A") %>% 
  dyShading(from = plot$time[start_is[8]], to = plot$time[stop_is[8]], color = "#FFE4C4")


data <- device_data %>% filter(device == "right")

start_is <- c()
stop_is <- c()
for(i in seq(8)){
  start_t <- reference_data$local_timestamp[start_index[i]]
  stop_t <- reference_data$local_timestamp[stop_index[i]]
  start_i <- which(data$local_timestamp > start_t)[1]-1
  stop_i <- which(data$local_timestamp > stop_t)[1]-1
  print(start_i)
  start_is <- c(start_is, start_i)
  stop_is <- c(stop_is, stop_i)
  print(stop_i)
  data$pose[start_i:stop_i] = pose[i]
  data$condition[start_i:stop_i] = condition[i]
}
start_is[1]
plot$time[start_is[1]]

plot <- data  %>% mutate(time=local_timestamp/1000000) %>% select(time, sti)
  p3 <- dygraph(plot, main = "right", xlab = "time (in seconds)", ylab = "STI", group = "sti_plots", width = w, height = h) %>%
  dyOptions(drawPoints = F, pointSize = 2) %>% 
    dyShading(from = plot$time[start_is[1]], to = plot$time[stop_is[1]], color = "#F0FFFF") %>% 
    dyShading(from = plot$time[start_is[2]], to = plot$time[stop_is[2]], color = "#FFE1FF") %>% 
    dyShading(from = plot$time[start_is[3]], to = plot$time[stop_is[3]], color = "#C1FFC1") %>% 
    dyShading(from = plot$time[start_is[4]], to = plot$time[stop_is[4]], color = "#FFF8DC") %>% 
    dyShading(from = plot$time[start_is[5]], to = plot$time[stop_is[5]], color = "#BFEFFF") %>% 
    dyShading(from = plot$time[start_is[6]], to = plot$time[stop_is[6]], color = "#FFBBFF") %>% 
    dyShading(from = plot$time[start_is[7]], to = plot$time[stop_is[7]], color = "#9AFF9A") %>% 
    dyShading(from = plot$time[start_is[8]], to = plot$time[stop_is[8]], color = "#FFE4C4")

library(htmlwidgets)
library(htmltools)

p2 <- div(style = "margin-top: 50px;", p2)
p3 <- div(style = "margin-top: 50px;", p3)
combined_dygraphs <- appendContent(p1, p2, p3)

# Display the combined dygraphs plots
combined_dygraphs

