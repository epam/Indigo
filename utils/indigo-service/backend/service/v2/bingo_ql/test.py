import unittest

from .query import QueryBuilder


class TestBingoQL(unittest.TestCase):
    def setUp(self):
        self.builder = QueryBuilder()

    def testQueryByPropName(self):
        query = self.builder.build_query('"monoisotopic_weight"')
        self.assertEquals(u"(elems->>'y' = %(property_term_0)s)", query)
        self.assertEquals(
            {"property_term_0": u"monoisotopic_weight"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('"BBB log([brain]:[blood])"')
        self.assertEquals(u"(elems->>'y' = %(property_term_0)s)", query)
        self.assertEquals(
            {"property_term_0": u"bbb log([brain]:[blood])"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('~"count"')
        self.assertEquals(u"(elems->>'y' LIKE %(property_term_0)s)", query)
        self.assertEquals(
            {"property_term_0": u"%count%"}, self.builder.bind_params
        )

        query = self.builder.build_query("count")
        self.assertEquals(u"(elems->>'y' LIKE %(property_term_0)s)", query)
        self.assertEquals(
            {"property_term_0": u"%count%"}, self.builder.bind_params
        )

    def testQueryPropWithValue(self):
        query = self.builder.build_query('"atom_count" != 30')
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float != %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": u"atom_count", "property_value_0": "30"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('"weight" > 0.537')
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float > %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": u"weight", "property_value_0": "0.537"},
            self.builder.bind_params,
        )

        query = self.builder.build_query("count > 25")
        self.assertEquals(
            u"(elems->>'x' LIKE %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float > %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": u"%count%", "property_value_0": "25"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('"formula" = "C14H21N3O2"')
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND elems->>'y' = %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": u"formula", "property_value_0": "c14h21n3o2"},
            self.builder.bind_params,
        )

        query = self.builder.build_query("'formula' != " + '"C14H21N3O2"')
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND elems->>'y' != %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {"property_term_0": u"formula", "property_value_0": "c14h21n3o2"},
            self.builder.bind_params,
        )

        query = self.builder.build_query('~"molecular formula" = "C14H21N3O2"')
        self.assertEquals(
            u"(elems->>'x' LIKE %(property_term_0)s AND elems->>'y' = %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"%molecular formula%",
                "property_value_0": "c14h21n3o2",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query('formula = "C14H21N3O2"')
        self.assertEquals(
            u"(elems->>'x' LIKE %(property_term_0)s AND elems->>'y' = %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"%formula%",
                "property_value_0": "c14h21n3o2",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query("'formula' ~ 'C14H21N3O2'")
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND elems->>'y' LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"formula",
                "property_value_0": "%c14h21n3o2%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query("formula !~ C14H21N3O2")
        self.assertEquals(
            u"(elems->>'x' LIKE %(property_term_0)s AND elems->>'y' NOT LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"%formula%",
                "property_value_0": "%c14h21n3o2%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query('"P-gp category_Probability" ~ "no"')
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND elems->>'y' LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"p-gp category_probability",
                "property_value_0": "%no%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query(
            '"PPB90 category_Probability" ~ "high = 0.18;"'
        )
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND elems->>'y' LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"ppb90 category_probability",
                "property_value_0": u"%high = 0.18;%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query('"molecular_formula" !~ "C14H21N3O2"')
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND elems->>'y' NOT LIKE %(property_value_0)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"molecular_formula",
                "property_value_0": u"%c14h21n3o2%",
            },
            self.builder.bind_params,
        )

    def testQueryCompound(self):
        query = self.builder.build_query(
            '"mass" > 30 OR ~"probability" !~ "LOW"'
        )
        self.assertEquals(
            u"(elems->>'x' = %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float > %(property_value_0)s) OR (elems->>'x' LIKE %(property_term_1)s AND elems->>'y' NOT LIKE %(property_value_1)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"mass",
                "property_value_0": u"30",
                "property_term_1": u"%probability%",
                "property_value_1": u"%low%",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query('"STATUS" or ~"NAME" or "CODE"')
        self.assertEquals(
            u"(elems->>'y' = %(property_term_0)s) OR (elems->>'y' LIKE %(property_term_1)s) OR (elems->>'y' = %(property_term_2)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"status",
                "property_term_1": u"%name%",
                "property_term_2": u"code",
            },
            self.builder.bind_params,
        )

        query = self.builder.build_query("logP > 2 and StdDev < 0.5")
        self.assertEquals(
            u"(elems->>'x' LIKE %(property_term_0)s AND jsonb_typeof(elems->'y') = 'number' AND (elems->>'y')::float > %(property_value_0)s))\n                    inner join {1} t1 on str.s = t1.s\n                    inner join jsonb_array_elements(t1.p) elems_t1 on ((elems_t1->>'x' LIKE %(property_term_1)s AND jsonb_typeof(elems_t1->'y') = 'number' AND (elems_t1->>'y')::float < %(property_value_1)s)",
            query,
        )
        self.assertEquals(
            {
                "property_term_0": u"%logp%",
                "property_term_1": u"%stddev%",
                "property_value_0": u"2",
                "property_value_1": u"0.5",
            },
            self.builder.bind_params,
        )


if __name__ == "__main__":
    unittest.main()
