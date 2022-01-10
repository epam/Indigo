from sklearn.cluster import SpectralClustering


def clustering(
    assay_values: list[list], method=SpectralClustering, **kwargs
) -> list:
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
    cl.fit(assay_values)
    return [label for label in cl.labels_]
