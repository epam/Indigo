import pytest
from sklearn.cluster import KMeans  # type: ignore

from indigo.ml.clustering import (
    average_distances,
    clustering,
    kmeans_cluster_center,
    split_coords_by_clusters,
)


@pytest.mark.parametrize(
    "args, kwargs, expecting",
    [
        (
            [[[1, 1], [2, 1], [1, 0], [4, 7], [3, 5]]],
            {
                "n_clusters": 2,
                "assign_labels": "discretize",
                "random_state": 0,
            },
            [0, 0, 0, 1, 1],
        ),
        (
            [[[1], [2], [1], [4], [3]]],
            {
                "n_clusters": 2,
                "assign_labels": "discretize",
                "random_state": 0,
            },
            [0, 0, 0, 1, 1],
        ),
    ],
)
def test_clustering_spectral(args, kwargs, expecting):
    assert clustering(*args, **kwargs) == expecting


@pytest.mark.parametrize(
    "coords, expecting",
    [
        (
            [[1, 1], [2, 1]],
            [1.5, 1.0],
        ),
    ],
)
def test_kmeans_cluster_center(coords, expecting):
    assert kmeans_cluster_center(coords) == expecting


@pytest.mark.parametrize(
    "coords, clusters, n_clusters, expected",
    [
        (
            [[1, 1], [4, 7], [2, 1], [3, 5]],
            [0, 1, 0, 1],
            2,
            [[[1, 1], [2, 1]], [[4, 7], [3, 5]]],
        )
    ],
)
def test_split_coords_by_clusters(coords, clusters, n_clusters, expected):
    assert split_coords_by_clusters(coords, clusters, n_clusters) == expected


@pytest.mark.parametrize(
    "cluster_centers, coords, expected",
    [
        (
            [[1.0, 1.0], [-2, -2]],
            [[[3, 1], [0, 1], [1, 1]], [[0, -2], [-2, -3]]],
            [1, 1.5],
        )
    ],
)
def test_average_distances(cluster_centers, coords, expected):
    assert average_distances(cluster_centers, coords) == expected
