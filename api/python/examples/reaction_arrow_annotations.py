from indigo import MyReaction


def main() -> None:
    rxn = MyReaction(
        smiles="c1ccccc1Br.CBr.[Na].[Na]>>Cc1ccccc1.[Na]Br.[Na]Br",
        catalyst=["Na", "Dry Ether"],
        temperature="150",
        pressure="20",
    )
    rxn.render("reaction_arrow_annotations.svg")


if __name__ == "__main__":
    main()