import unittest

from .query import QueryBuilder


class TestBingoQL(unittest.TestCase):
    def setUp(self):
        self.builder = QueryBuilder()

    def testQueryByPropName(self):
        query = self.builder.build_query('"monoisotopic_weight"')
        self.assertEquals("(elems->>'y' = %(property_term_0)s)", query)
        self.assertEquals(
            {"property_term_0": "monoisotopic_weight"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('"BBB log([brain]:[blood])"')
        self.assertEquals("(elems->>'y' = %(property_term_0)s)", query)
        self.assertEquals(
            {"property_term_0": "bbb log([brain]:[blood])"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('~"count"')
        self.assertEquals("(elems->>'y' LIKE %(property_term_0)s)", query)
        self.assertEquals(
            {"property_term_0": "%count%"}, self.builder.bind_params
        )

        query = self.builder.build_query("count")
        self.assertEquals("(elems->>'y' LIKE %(property_term_0)s)", query)
        self.assertEquals(
            {"property_term_0": "%count%"}, self.builder.bind_params
        )

    def testQueryPropWithValue(self):
        query = self.builder.build_query('"atom_count" != 30')
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float != %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": "atom_count", "property_value_0": "30"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('"weight" > 0.537')
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float > %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": "weight", "property_value_0": "0.537"},
            self.builder.bind_params,
        )

        query = self.builder.build_query("count > 25")
        self.assertEquals(
            "(elems->>'x' LIKE %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float > %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": "%count%", "property_value_0": "25"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('"formula" = "C14H21N3O2"')
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND elems->>'y' = %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": "formula", "property_value_0": "c14h21n3o2"},
            self.builder.bind_params,
        )

        query = self.builder.build_query("'formula' != " + '"C14H21N3O2"')
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND elems->>'y' != %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": "formula", "property_value_0": "c14h21n3o2"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('~"molecular formula" = "C14H21N3O2"')
        self.assertEquals(
            "(elems->>'x' LIKE %(property_term_0)s AND elems->>'y' = %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "%molecular formula%",
                "property_value_0": "c14h21n3o2",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query('formula = "C14H21N3O2"')
        self.assertEquals(
            "(elems->>'x' LIKE %(property_term_0)s AND elems->>'y' = %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "%formula%",
                "property_value_0": "c14h21n3o2",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query("'formula' ~ 'C14H21N3O2'")
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND elems->>'y' LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "formula",
                "property_value_0": "%c14h21n3o2%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query("formula !~ C14H21N3O2")
        self.assertEquals(
            "(elems->>'x' LIKE %(property_term_0)s AND elems->>'y' NOT LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "%formula%",
                "property_value_0": "%c14h21n3o2%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query('"P-gp category_Probability" ~ "no"')
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND elems->>'y' LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "p-gp category_probability",
                "property_value_0": "%no%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query(
            '"PPB90 category_Probability" ~ "high = 0.18;"'
        )
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND elems->>'y' LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "ppb90 category_probability",
                "property_value_0": "%high = 0.18;%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query('"molecular_formula" !~ "C14H21N3O2"')
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND elems->>'y' NOT LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "molecular_formula",
                "property_value_0": "%c14h21n3o2%",
            },
            self.builder.bind_params,
        )

    def testQueryCompound(self):
        query = self.builder.build_query(
            '"mass" > 30 OR ~"probability" !~ "LOW"'
        )
        self.assertEquals(
            "(elems->>'x' = %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float > %(property_value_0)s) OR (elems->>'x' LIKE %(property_term_1)s AND elems->>'y' NOT LIKE %(property_value_1)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "mass",
                "property_value_0": "30",
                "property_term_1": "%probability%",
                "property_value_1": "%low%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query('"STATUS" or ~"NAME" or "CODE"')
        self.assertEquals(
            "(elems->>'y' = %(property_term_0)s) OR (elems->>'y' LIKE %(property_term_1)s) OR (elems->>'y' = %(property_term_2)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "status",
                "property_term_1": "%name%",
                "property_term_2": "code",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query("logP > 2 and StdDev < 0.5")
        self.assertEquals(
            "(elems->>'x' LIKE %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float > %(property_value_0)s))\n                    inner join {1} t1 on str.s = t1.s\n                    inner join jsonb_array_elements(t1.p) elems_t1 on ((elems_t1->>'x' LIKE %(property_term_1)s AND jsonb_typeof(elems_t1->'y') = 'number' AND (elems_t1->>'y')::float < %(property_value_1)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": "%logp%",
                "property_term_1": "%stddev%",
                "property_value_0": "2",
                "property_value_1": "0.5",
            },
            self.builder.bind_params,
        )


if __name__ == "__main__":
    unittest.main()
