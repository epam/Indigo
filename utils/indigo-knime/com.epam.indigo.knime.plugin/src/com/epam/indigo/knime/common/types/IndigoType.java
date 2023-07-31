package com.epam.indigo.knime.common.types;

import org.knime.core.data.AdapterCell;
import org.knime.core.data.DataCell;
import org.knime.core.data.DataType;
import org.knime.core.data.DataValue;
import org.knime.core.data.RWAdapterValue;
import org.knime.core.data.StringValue;
import org.knime.core.data.def.StringCell.StringCellFactory;
import org.knime.chem.types.InchiCellFactory;
import org.knime.chem.types.InchiValue;
import org.knime.chem.types.MolCellFactory;
import org.knime.chem.types.MolValue;
import org.knime.chem.types.RxnCellFactory;
import org.knime.chem.types.RxnValue;
import org.knime.chem.types.SdfCellFactory;
import org.knime.chem.types.SdfValue;
import org.knime.chem.types.SmartsCellFactory;
import org.knime.chem.types.SmartsValue;
import org.knime.chem.types.SmilesCellFactory;
import org.knime.chem.types.SmilesValue;
import org.knime.chem.types.CMLCellFactory;
import org.knime.chem.types.CMLValue;

import com.epam.indigo.Indigo;
import com.epam.indigo.IndigoException;
import com.epam.indigo.IndigoInchi;
import com.epam.indigo.IndigoObject;
import com.epam.indigo.knime.cell.IndigoDataCell;
import com.epam.indigo.knime.cell.IndigoMolAdapterCell;
import com.epam.indigo.knime.cell.IndigoMolCell;
import com.epam.indigo.knime.cell.IndigoMolCellFactory;
import com.epam.indigo.knime.cell.IndigoMolValue;
import com.epam.indigo.knime.cell.IndigoQueryMolAdapterCell;
import com.epam.indigo.knime.cell.IndigoQueryMolCell;
import com.epam.indigo.knime.cell.IndigoQueryMolCellFactory;
import com.epam.indigo.knime.cell.IndigoQueryMolValue;
import com.epam.indigo.knime.cell.IndigoQueryReactionAdapterCell;
import com.epam.indigo.knime.cell.IndigoQueryReactionCell;
import com.epam.indigo.knime.cell.IndigoQueryReactionCellFactory;
import com.epam.indigo.knime.cell.IndigoQueryReactionValue;
import com.epam.indigo.knime.cell.IndigoReactionAdapterCell;
import com.epam.indigo.knime.cell.IndigoReactionCell;
import com.epam.indigo.knime.cell.IndigoReactionCellFactory;
import com.epam.indigo.knime.cell.IndigoReactionValue;
import com.epam.indigo.knime.plugin.IndigoPlugin;

/**
 * As turned out the easiest way to store indigo's types and all their required
 * operations is ENUM. Enumerations help to avoid tons of duplicating switch-
 * and if- blocks.
 *
 */
public enum IndigoType implements IndigoTypeProperties {

	@SuppressWarnings("unchecked") 
	MOLECULE(new Class[] { MolValue.class, SdfValue.class, SmilesValue.class,
			CMLValue.class, InchiValue.class, StringValue.class }, IndigoMolAdapterCell.RAW_TYPE, IndigoMolValue.class,
			STRING_MOLECULE) {

		@Override
		public AdapterCell createAdapterFromIndigoDataCell(IndigoDataCell source) {
			return new IndigoMolAdapterCell((IndigoMolCell) source);
		}

		@Override
		public DataCell createAdapterContainingIndigoRepr(DataCell source, boolean isAdapter, boolean stringAsSMARTS) {

			if (source.isMissing())
				throw new IndigoException(source, "coud not process a missing cell.");

			Indigo indigo = IndigoPlugin.getIndigo();
			IndigoInchi inchi = IndigoPlugin.getIndigoInchi();

			DataType cellType = source.getType();
			boolean isInchi = false;
			String repr = "";
			RWAdapterValue adapter = null;

			if (StringCellFactory.TYPE.equals(cellType))
				return source;

			if (cellType.isCompatible(MolValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(MolValue.class).getMolValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((MolValue) source).getMolValue();
					adapter = (RWAdapterValue) MolCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(SdfValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(SdfValue.class).getSdfValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((SdfValue) source).getSdfValue();
					adapter = (RWAdapterValue) SdfCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(SmilesValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(SmilesValue.class).getSmilesValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((SmilesValue) source).getSmilesValue();
					adapter = (RWAdapterValue) SmilesCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(CMLValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(CMLValue.class).getCMLValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((CMLValue) source).getCMLValue();
					adapter = (RWAdapterValue) CMLCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(InchiValue.class)) {
				isInchi = true;

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(InchiValue.class).getInchiString();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((InchiValue) source).getInchiString();
					adapter = (RWAdapterValue) InchiCellFactory.createAdapterCell(repr);
				}

			}

			if (adapter == null && !cellType.isCompatible(StringValue.class))
				throw new RuntimeException("Cannot find a compatible value class.");

			IndigoObject io = isInchi ? inchi.loadMolecule(repr) : indigo.loadMolecule(repr);

			return adapter.cloneAndAddAdapter(IndigoMolCellFactory.createAdapterCell(io), IndigoMolValue.class);

		}

	},

	@SuppressWarnings("unchecked") 
	REACTION(
			new Class[] { RxnValue.class, SmilesValue.class, CMLValue.class, StringValue.class },
			IndigoReactionAdapterCell.RAW_TYPE, IndigoReactionValue.class, STRING_REACTION) {

		@Override
		public AdapterCell createAdapterFromIndigoDataCell(IndigoDataCell source) {
			return new IndigoReactionAdapterCell((IndigoReactionCell) source);
		}

		@Override
		public DataCell createAdapterContainingIndigoRepr(DataCell source, boolean isAdapter, boolean stringAsSMARTS) {

			if (source.isMissing())
				throw new IndigoException(source, "coud not process a missing cell.");

			Indigo indigo = IndigoPlugin.getIndigo();

			DataType cellType = source.getType();
			String repr = "";
			RWAdapterValue adapter = null;

			if (StringCellFactory.TYPE.equals(cellType))
				return source;

			if (cellType.isCompatible(SmilesValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(SmilesValue.class).getSmilesValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((SmilesValue) source).getSmilesValue();
					adapter = (RWAdapterValue) SmilesCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(CMLValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(CMLValue.class).getCMLValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((CMLValue) source).getCMLValue();
					adapter = (RWAdapterValue) CMLCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(RxnValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(RxnValue.class).getRxnValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((RxnValue) source).getRxnValue();
					adapter = (RWAdapterValue) RxnCellFactory.createAdapterCell(repr);
				}

			}

			if (adapter == null)
				throw new RuntimeException("Cannot find a compatible value class.");

			IndigoObject io = indigo.loadReaction(repr);

			return adapter.cloneAndAddAdapter(IndigoReactionCellFactory.createAdapterCell(io),
					IndigoReactionValue.class);
		}

	},

	@SuppressWarnings("unchecked") 
	QUERY_MOLECULE(
			new Class[] { SmartsValue.class, MolValue.class, SdfValue.class, SmilesValue.class, StringValue.class },
			IndigoQueryMolAdapterCell.RAW_TYPE, IndigoQueryMolValue.class, STRING_QUERY_MOLECULE) {

		@Override
		public AdapterCell createAdapterFromIndigoDataCell(IndigoDataCell source) {
			return new IndigoQueryMolAdapterCell((IndigoQueryMolCell) source);
		}

		@Override
		public DataCell createAdapterContainingIndigoRepr(DataCell source, boolean isAdapter, boolean stringAsSMARTS) {

			if (source.isMissing())
				throw new IndigoException(source, "coud not process a missing cell.");

			DataType cellType = source.getType();
			String repr = "";
			RWAdapterValue adapter = null;

			if (StringCellFactory.TYPE.equals(cellType) && !stringAsSMARTS)
				return source;

			if (cellType.isCompatible(SmartsValue.class)) {
				stringAsSMARTS = true;

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(SmartsValue.class).getSmartsValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((SmartsValue) source).getSmartsValue();
					adapter = (RWAdapterValue) SmartsCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(MolValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(MolValue.class).getMolValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((MolValue) source).getMolValue();
					adapter = (RWAdapterValue) MolCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(SdfValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(SdfValue.class).getSdfValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((SdfValue) source).getSdfValue();
					adapter = (RWAdapterValue) SdfCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(SmilesValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(SmilesValue.class).getSmilesValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((SmilesValue) source).getSmilesValue();
					adapter = (RWAdapterValue) SmilesCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(StringValue.class)) {
				repr = ((StringValue) source).getStringValue();
				adapter = (RWAdapterValue) SmartsCellFactory.createAdapterCell(repr);
			}

			if (adapter == null)
				throw new RuntimeException("Cannot find a compatible value class.");

			return adapter.cloneAndAddAdapter(IndigoQueryMolCellFactory.createAdapterCell(repr, stringAsSMARTS),
					IndigoQueryMolValue.class);
		}

	},

	@SuppressWarnings("unchecked") 
	QUERY_REACTION(
			new Class[] { SmartsValue.class, RxnValue.class, SmilesValue.class, StringValue.class },
			IndigoQueryReactionAdapterCell.RAW_TYPE, IndigoQueryReactionValue.class, STRING_QUERY_REACTION) {

		@Override
		public AdapterCell createAdapterFromIndigoDataCell(IndigoDataCell source) {
			return new IndigoQueryReactionAdapterCell((IndigoQueryReactionCell) source);
		}

		@Override
		public DataCell createAdapterContainingIndigoRepr(DataCell source, boolean isAdapter, boolean stringAsSMARTS) {

			if (source.isMissing())
				throw new IndigoException(source, "coud not process a missing cell.");

			DataType cellType = source.getType();
			String repr = "";
			RWAdapterValue adapter = null;

			if (StringCellFactory.TYPE.equals(cellType) && !stringAsSMARTS)
				return source;

			if (cellType.isCompatible(SmartsValue.class)) {
				stringAsSMARTS = true;

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(SmartsValue.class).getSmartsValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((SmartsValue) source).getSmartsValue();
					adapter = (RWAdapterValue) SmartsCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(SmilesValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(SmilesValue.class).getSmilesValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((SmilesValue) source).getSmilesValue();
					adapter = (RWAdapterValue) SmilesCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(RxnValue.class)) {

				if (isAdapter) {
					repr = ((AdapterCell) source).getAdapter(RxnValue.class).getRxnValue();
					adapter = (RWAdapterValue) source;
				} else {
					repr = ((RxnValue) source).getRxnValue();
					adapter = (RWAdapterValue) RxnCellFactory.createAdapterCell(repr);
				}

			} else if (cellType.isCompatible(StringValue.class)) {

				repr = ((StringValue) source).getStringValue();
				adapter = (RWAdapterValue) SmartsCellFactory.createAdapterCell(repr);

			}

			if (adapter == null)
				throw new RuntimeException("Cannot find a compatible value class.");

			return adapter.cloneAndAddAdapter(IndigoQueryReactionCellFactory.createAdapterCell(repr, stringAsSMARTS),
					IndigoQueryReactionValue.class);
		}

	};

	private Class<? extends DataValue>[] convertableTypes;
	private DataType indigoDataType;
	private Class<? extends DataValue> indigoDataValueClass;
	private String stringValue;

	private IndigoType(Class<? extends DataValue>[] convertableTypes, DataType indigoDataType,
			Class<? extends DataValue> indigoDataValueClass, String stringValue) {
		this.convertableTypes = convertableTypes;
		this.indigoDataType = indigoDataType;
		this.indigoDataValueClass = indigoDataValueClass;
		this.stringValue = stringValue;
	}

	@Override
	public DataType getIndigoDataType() {
		return indigoDataType;
	}

	@Override
	public Class<? extends DataValue> getIndigoDataValueClass() {
		return indigoDataValueClass;
	}

	@Override
	public Class<? extends DataValue>[] getClassesConvertableToIndigoDataClass() {
		return convertableTypes;
	}

	@Override
	public String toString() {
		return stringValue;
	}

	public static IndigoType findByString(String str) {
		for (IndigoType type : values()) {
			if (type.toString().equals(str))
				return type;
		}
		return null;
	}

}
