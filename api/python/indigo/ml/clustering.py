from typing import Type, Union

from sklearn.base import ClusterMixin
from sklearn.cluster import SpectralClustering


def clustering(
    assay_values: list[list[Union[int, float]]],
    method: Type[ClusterMixin] = SpectralClustering,
    **kwargs,
) -> list[int]:
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
