# Indigo tests #

Directory with tests: `indigo-tests/bingo/tests/`

Tests with bingo-nosql:
 * bigtable
 * exact
 * markush
 * mass
 * pseudoatoms
 * resonance
 * rexact
 * rsmarts
 * rsub
 * sgroups
 * similarity
 * smarts
 * substructure
 * tautomers

Tests with bingo-elastic:
 * exact
 * similarity
 * substructure

All tests have PostgreSQL support

## Tests structure ##
* tests/
  - data/ - Contains queries, targets data and test cases
  - dbc/ - Contains DB adapters: PostgreSQL, Bingo NOSQL, Bingo Elastic
  - constants.py 
  - helpers.py - Set of useful functions
  - logger.py 
  - db_config.ini - DB connection parameters  
  - req.txt - Requirement modules
  - bingo-tests.log - Logs (appears after running tests)
  - conftest.py - Contains pytest fixtures: indigo instance, DB connection
  - ...
  - test_{FUNCTION}/
    * conftest.py - Setup/teardown scripts, molecules/reactions mapping: id - entity  
    * test_{FUNCTION}.py  
  - ...
   
## Test example ##


```python
# test_function.py
import pytest

from ..constants import DB_POSTGRES, DB_BINGO, DB_BINGO_ELASTIC
from ..helpers import assert_calculate_query, query_cases

# query_cases - return list of tuples: test cases with expected test result. 
# [(query_id, expected), ]

# assert_calculate_query - Assertion function

db_list = [DB_POSTGRES, DB_BINGO, DB_BINGO_ELASTIC] # List of Databases we run tests in 

@pytest.mark.usefixtures('init_db') # pytest fixture (setup/teardown) defined in coftest.py (near test_function.py)
class TestFunction:
    # Parametrize parameter 'db' in function test_foo with arguments from 'db_list', so the function will run with different 'db' values
    # 'db' is pytest fixture provided in root conftest.py file
    # Parametrize parameters 'query_id' and 'expected' in function test_foo with query_cases, so the function will run with different 'query_id' and 'expected' values
    @pytest.mark.parametrize('db', db_list, indirect=True)
    @pytest.mark.parametrize('query_id, expected', query_cases('function'))
    def test_foo(self, db, entities, query_id, expected):
        # entities - pytest fixture defined in conftest.py (near test_function.py)
        molecule = entities.get(query_id)
        result = db.function(molecule, 'bar-option')
        assert_calculate_query(result, expected)

# Double parametrization does let us run test_foo with different test cases 
# for every database provided in 'db_list'    
```
   

## Install requirements ##
```shell
pip install -r indigo-tests/bingo/tests/requirements.txt
``` 


## Run docker elastic ##

```shell
docker run -p 9200:9200 --env "discovery.type=single-node" --env "indices.query.bool.max_clause_count=4096" --env "opendistro_security.disabled=true" amazon/opendistro-for-elasticsearch:latest
```

## Running tests ##
```shell
cd indigo-tests/bingo/tests
pytest .
```

