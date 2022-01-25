from typing import List, Type, Union

import umap  # type: ignore
from sklearn.base import BaseEstimator  # type: ignore


def reduce_dim(
    descriptors: List[List[Union[int, float]]],
    method: Type[BaseEstimator] = umap.UMAP,
    **kwargs
) -> List[List[float]]:
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
