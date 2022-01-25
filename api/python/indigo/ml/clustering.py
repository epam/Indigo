from typing import List, Type, Union

from sklearn.base import ClusterMixin  # type: ignore
from sklearn.cluster import SpectralClustering  # type: ignore


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
