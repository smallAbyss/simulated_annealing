import matplotlib.pyplot as plt

# Read data from the file
data = []

k = 0
with open("./stats/delta_ans.txt", "r") as file:
    for line in file:
        k += 1
        if k % 1000 != 0: 
            continue
        
        line = line.strip()
        if not line:
            continue
        
        x, y = map(float, line.split())
        data.append((x, y))


# Split coordinates into separate lists
x_values = [point[0] for point in data]
y_values = [point[1] for point in data]

# Draw the graph
plt.plot(x_values, y_values, marker="o")

plt.xlabel("x")
plt.ylabel("y")
plt.title("Graph from data.txt")
plt.grid(True)

plt.show()
