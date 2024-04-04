# Written by Raj Pandya
# For plotting the map on open3d

import open3d as o3d
import numpy as np

point_cloud = o3d.io.read_point_cloud("output.xyz")
k = 10

kdtree = o3d.geometry.KDTreeFlann(point_cloud)
lines = []
for i in range(len(point_cloud.points)):
        [k, idx, _] = kdtree.search_knn_vector_3d(point_cloud.points[i], k)
        for j in idx:
            if j != i:
                lines.append([i, j])

line_set = o3d.geometry.LineSet(
        points=o3d.utility.Vector3dVector(np.asarray(point_cloud.points)),
        lines=o3d.utility.Vector2iVector(lines),
)

o3d.visualization.draw_geometries([point_cloud, line_set])