from math import sqrt
from statistics import mean
from typing import List, Type, Union

from sklearn.base import ClusterMixin  # type: ignore
from sklearn.cluster import KMeans, SpectralClustering  # type: ignore


def clustering(
    assay_values: List[List[Union[int, float]]],
    method: Type[ClusterMixin] = SpectralClustering,
    **kwargs,
) -> List[int]:
    """Clustering procedure for given list of assay values.

    Args:
        assay_values: Ordered list of lists of assay values.
        method: clustering method, default: SpectralClustering.
    Kwargs:
        params for clustering method.
    Returns:
        ordered list of cluster labels.
    """
    cl = method(**kwargs)
    labels = cl.fit_predict(assay_values)
    return labels.tolist()


def kmeans_cluster_center(
    coordinates: List[List[Union[int, float]]], n_clusters: int = 1
) -> List[float]:
    cl = KMeans(n_clusters)
    cl.fit_predict(coordinates)
    return cl.cluster_centers_.reshape(-1).tolist()


def calculate_distance(p1: List[float], p2: List[float]):
    return sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2)


def average_distances(
    cluster_centers: List[List[float]], coordinates: List[List[List[float]]]
) -> List[float]:
    mean_distances = []
    for i, cluster_coords in enumerate(coordinates):
        distances = [
            calculate_distance(coords, cluster_centers[i])
            for coords in cluster_coords
        ]
        mean_distances.append(mean(distances))
    return mean_distances


def split_coords_by_clusters(
    coordinates: List[List[float]], clusters: List[int], n_clusters: int
) -> List[List[List[float]]]:
    clustered_coords = [[] for _ in range(n_clusters)]
    for i, cluster in enumerate(clusters):
        clustered_coords[cluster].append(coordinates[i])
    return clustered_coords
