# lslsub_dbfeeder

## Requierements
### Linux
- [PostreSQL 10](https://www.postgresql.org/download)
- [Timescaledb](https://docs.timescale.com/v0.9/getting-started/installation/linux/installation-apt-ubuntu)

You can also run :
```bash
./setup.sh
```

### Windows
- [PostreSQL 10](https://www.postgresql.org/download/windows/)
- [Timescaledb](https://docs.timescale.com/v0.9/getting-started/installation/windows/installation-windows)
- Run to configure the database
```bash
setup.bat
```

## Information
- **Brief**: Search for available LSL stream, for each of them create a thread doing the following: open the database, create a table and store the samples.
- **Documentation & Git**: [Doc link](https://aightech.github.io/lslsub_dbfeeder/html/index.html) & [Git link](https://github.com/Aightech/lslsub_dbfeeder)
- **Languages**: C++
- **Libraries**: LSL / pqxx
- **Note**: /
- **Compatibility**:

| Ubuntu           | Window10         | MacOS            |
|:----------------:|:----------------:|:----------------:|
|:heavy_check_mark:|:heavy_check_mark:|:grey_question:   |