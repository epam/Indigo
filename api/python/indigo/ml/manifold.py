from typing import Type

import umap
from sklearn.base import BaseEstimator  # type: ignore


def reduce_dim(
    descriptors: list[list[int, float]],
    method: Type[BaseEstimator] = umap.UMAP,
    **kwargs
) -> list[list[float]]:
    """Dimensional reduction procedure for given set of molecules.

    Args:
        descriptors: ordered list of molecule descriptors (fingerprints,
        feature vectors, other descriptors).
        method: dimensional reduction method, UMAP by default.
    Kwargs:
        Dimensional reduction method params.
    Returns:
        Ordered list with 2D coordinates.
    """
    embedding = method(**kwargs).fit_transform(descriptors)
    return embedding.tolist()
