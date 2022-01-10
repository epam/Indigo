from sklearn.cluster import SpectralClustering


def clustering(
    assay_values: list[list], method=SpectralClustering, **kwargs
) -> list:
    cl = method(**kwargs)
    cl.fit(assay_values)
    return [label for label in cl.labels_]
