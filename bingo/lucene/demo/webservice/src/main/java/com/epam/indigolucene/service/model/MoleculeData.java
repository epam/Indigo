package com.epam.indigolucene.service.model;


/**
 * Created by Artem Malykh on 20.06.16.
 */
public class MoleculeData {

    protected String moleculeId;
    protected String moleculeRepresentation;

    public MoleculeData(String moleculeId, String moleculeRepresentation) {
        this.moleculeId = moleculeId;
        this.moleculeRepresentation = moleculeRepresentation;
    }

    public String getMoleculeId() {
        return moleculeId;
    }

    public void setMoleculeId(String moleculeId) {
        this.moleculeId = moleculeId;
    }

    public String getMoleculeRepresentation() {
        return moleculeRepresentation;
    }

    public void setMoleculeRepresentation(String moleculeRepresentation) {
        this.moleculeRepresentation = moleculeRepresentation;
    }
}
