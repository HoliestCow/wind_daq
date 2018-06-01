
Synthetic data generation idea.
1) read allinone.csv second column line by line and write to ram.
2) once a rng condition hits True, histogram the ram data and write result in buffer.csv
3) buffer.csv is watched for fifo stuff by the CVRS. The result is written to the PTU's database.
4) The PTU database is read by the web app for online display
