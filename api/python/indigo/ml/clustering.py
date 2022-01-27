from typing import List, Type, Union, Tuple

from sklearn.base import ClusterMixin  # type: ignore
from sklearn.cluster import SpectralClustering, KMeans  # type: ignore


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


def kmeans_clustering(
    assay_values: List[List[Union[int, float]]], **kwargs
) -> Tuple[List[int], List[List[float]]]:
    """Clustering procedure with KMeans method.

    Args:
        assay_values: Ordered list of lists of assay values.
    Kwargs:
        params for KMeans clustering method.
    Returns:
        tuple with list of cluster labels and list of cluster centers
        coordinates.
    """
    cl = KMeans(**kwargs)
    labels = cl.fit_predict(assay_values)
    return labels.tolist(), cl.cluster_centers_.tolist()


def average_distance():
    pass


def split_coords_by_clusters(coordinates, clusters, n_clusters):
    sorted_coords = [[] for _ in range(n_clusters)]
    for i, cluster in enumerate(clusters):
        sorted_coords[cluster].append(coordinates[i])
    return sorted_coords
