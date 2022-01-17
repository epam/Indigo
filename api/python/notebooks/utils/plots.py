import base64
from typing import List

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
from bokeh.models import ColumnDataSource, HoverTool, Legend
from bokeh.plotting import figure, show

from indigo import Indigo
from indigo.renderer import IndigoRenderer

indigo = Indigo()
renderer = IndigoRenderer(indigo)
indigo.setOption("render-output-format", "svg")
indigo.setOption("render-image-size", 350, 170)


def molecule_image(smiles: str) -> str:
    mol = indigo.loadMolecule(smiles)
    mol.aromatize()
    svg = renderer.renderToString(mol)
    return (
        "data:image/svg+xml;base64," + base64.b64encode(svg.encode()).decode()
    )


def static_avp_avr_graphs(preds, actual, r2: str, title: str = ""):
    """Static prediction/actual and prediction/residual"""
    title = title
    x, y = pd.Series(actual, name="Actual"), pd.Series(
        preds, name="Prediction"
    )
    z = pd.Series(preds, name="Residual")
    fig, ax = plt.subplots(1, 2, figsize=(15, 5))
    sns.regplot(x=x, y=y, ax=ax[0])
    sns.residplot(x=x, y=z, ax=ax[1])
    ax[0].set_title(f"{title} Actual vs Predicted, r2={r2:.3f}")
    ax[1].set_title(f"Residual plot for {title}, r2={r2:.3f}")


def avp_plot(
    actual: List[float],
    predicted: List[float],
    ids: List[str],
    smiles_list: List[str],
    r2: float,
    title: str = "",
) -> None:
    df = pd.DataFrame({"x": actual, "y": predicted})
    df["id"] = ids
    df["image"] = [molecule_image(smiles) for smiles in smiles_list]
    df["residual"] = actual - predicted
    plot_height = 400
    plot_width = int(plot_height * 1.4)
    plot_figure = figure(
        title=f"{title} Actual vs Predicted, r2={r2:.3f}",
        plot_width=plot_width,
        plot_height=plot_height,
        x_axis_label="Actual",
        y_axis_label="Predicted",
        tools="pan, wheel_zoom, reset",
    )
    plot_figure.add_tools(
        HoverTool(
            tooltips="""
            <div>
            <div>
            <img src='@image' style='float: left; margin: 5px 5px 5px 5px'/>
            </div>
            <div>
            <span style='font-size: 16px; color: #224499'>Compound:</span>
            <span style='font-size: 18px'>@id</span>
            </div>
            <div>
            <span style='font-size: 16px; color: #224499'>Residual:</span>
            <span style='font-size: 18px'>@residual</span>
            </div>
            </div>
            """
        )
    )
    plot_figure.add_layout(Legend(), "right")
    datasource = ColumnDataSource(df)
    # pylint: disable=too-many-function-args
    plot_figure.circle(
        "x",
        "y",
        source=datasource,
        line_alpha=0.6,
        fill_alpha=0.6,
        size=4,
    )
    show(plot_figure)


def avr_plot(
    actual: List[float],
    predicted: List[float],
    ids: List[str],
    smiles_list: List[str],
    r2: float,
    title="",
) -> None:
    df = pd.DataFrame({"x": actual, "y": actual - predicted})
    df["id"] = ids
    df["image"] = [molecule_image(smiles) for smiles in smiles_list]
    df["residual"] = actual - predicted
    plot_height = 400
    plot_width = int(plot_height * 1.4)
    plot_figure = figure(
        title=f"{title} Actual vs Residual, r2={r2:.3f}",
        plot_width=plot_width,
        plot_height=plot_height,
        x_axis_label="Actual",
        y_axis_label="Residual",
        tools="pan, wheel_zoom, reset",
    )
    plot_figure.add_tools(
        HoverTool(
            tooltips="""
            <div>
            <div>
            <img src='@image' style='float: left; margin: 5px 5px 5px 5px'/>
            </div>
            <div>
            <span style='font-size: 16px; color: #224499'>Compound:</span>
            <span style='font-size: 18px'>@id</span>
            </div>
            <div>
            <span style='font-size: 16px; color: #224499'>Residual:</span>
            <span style='font-size: 18px'>@residual</span>
            </div>
            </div>
            """
        )
    )
    plot_figure.add_layout(Legend(), "right")
    datasource = ColumnDataSource(df)
    # pylint: disable=too-many-function-args
    plot_figure.circle(
        "x",
        "y",
        source=datasource,
        line_alpha=0.6,
        fill_alpha=0.6,
        size=4,
    )
    show(plot_figure)
