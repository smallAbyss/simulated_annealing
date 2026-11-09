# Simulated annealing for searching Global optimum


## Fast start
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build 
./build/sa
```

## Statistic

- Count succesfull-solved tasks
    - `python scripts/python/stat-collect.py ./data/stats/ans.csv`

- Draw a a function 
    - `python scripts/python/draw-func.py data/grids/gkls_1.csv`

- Collect points for grids
    - `./build/grid 1 100 1000 1000 ./data/grids`
    - `./build/grid <from> <to>  <grid_height> <grid_width> <out_folder>`
