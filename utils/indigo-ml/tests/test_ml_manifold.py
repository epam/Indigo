import pytest

from indigo.ml.manifold import reduce_dim


@pytest.mark.parametrize(
    "args, kwargs",
    [
        (
            [
                [
                    [3, 0, 0, 77, 70, 218, 234, 87, 218, 228],
                    [15, 3, 0, 255, 254, 254, 254, 253, 63, 251],
                    [7, 0, 0, 255, 95, 191, 254, 253, 122, 255],
                    [7, 0, 0, 252, 223, 214, 239, 221, 94, 254],
                ]
            ],
            {
                "random_state": 42,
                "min_dist": 0.1,
                "n_components": 2,
                "metric": "euclidean",
            },
        ),
    ],
)
def test_reduce_dim(args, kwargs):
    emb = reduce_dim(*args, **kwargs)
    assert len(emb[0]) == 2 and len(emb) == len(args[0])
