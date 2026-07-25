import json
import re
from typing import List, Optional, Sequence, Union

from .indigo.indigo import Indigo
from .renderer.renderer import IndigoRenderer


class MyReaction:
    ANNOTATION_PROPERTY = "indigo:reaction-arrow-annotations"

    def __init__(
        self,
        smiles: str,
        catalyst: Optional[Union[str, Sequence[str]]] = None,
        temperature: Optional[Union[str, int, float]] = None,
        pressure: Optional[Union[str, int, float]] = None,
        session: Optional[Indigo] = None,
    ) -> None:
        self._session = session or Indigo()
        self._renderer = IndigoRenderer(self._session)
        self._reaction = self._session.loadReaction(smiles)
        self._set_annotations(
            catalyst=catalyst,
            temperature=temperature,
            pressure=pressure,
        )
        self._reaction.layout()

    @staticmethod
    def _normalize_lines(
        value: Optional[Union[str, Sequence[str]]]
    ) -> List[str]:
        if value is None:
            return []
        if isinstance(value, str):
            items = [value]
        else:
            items = [str(item) for item in value]
        lines: List[str] = []
        for item in items:
            text = str(item).strip()
            if text:
                lines.append(text)
        return lines

    @staticmethod
    def _format_temperature(
        value: Optional[Union[str, int, float]]
    ) -> Optional[str]:
        if value is None:
            return None
        text = str(value).strip()
        if re.fullmatch(r"-?\d+(?:\.\d+)?", text):
            return f"{text}°C"
        return text

    @staticmethod
    def _format_pressure(
        value: Optional[Union[str, int, float]]
    ) -> Optional[str]:
        if value is None:
            return None
        text = str(value).strip()
        if re.fullmatch(r"-?\d+(?:\.\d+)?", text):
            return f"{text} psi"
        return text

    def _set_annotations(
        self,
        catalyst: Optional[Union[str, Sequence[str]]],
        temperature: Optional[Union[str, int, float]],
        pressure: Optional[Union[str, int, float]],
    ) -> None:
        top = self._normalize_lines(catalyst)
        bottom: List[str] = []

        formatted_temperature = self._format_temperature(temperature)
        if formatted_temperature:
            bottom.append(formatted_temperature)

        formatted_pressure = self._format_pressure(pressure)
        if formatted_pressure:
            bottom.append(formatted_pressure)

        if not top and not bottom:
            return

        payload = {}
        if top:
            payload["top"] = top
        if bottom:
            payload["bottom"] = bottom
        self._reaction.setProperty(
            self.ANNOTATION_PROPERTY,
            json.dumps(payload, ensure_ascii=False),
        )

    def render(self, filename: str) -> None:
        self._renderer.renderToFile(self._reaction, filename)

    def render_to_string(self) -> str:
        return self._renderer.renderToString(self._reaction)

    @property
    def reaction(self):
        return self._reaction
