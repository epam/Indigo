- Start Date: 2015-11-03
- Implementation MR: !4
- Source issue: #47
- Status: Implemented
- Scope: db, optimization

# Summary

Add ```index_data``` column to ```library_metadata``` table.

# Motivation

Right now ```library_metadata.service_data``` *jsonb* column is being populated
with **properties** field when a client uploads SDF collection to a service. We
need this information for the autocomplete feature of our web-client, and
**properties** are dispatched along with other library information:

```
$ curl -s localhost:5000/v1/libraries/9ca31f91-2f5a-4c01-adb4-4fc137ff8a4f
{
   "service_data" : {
      "properties" : [
         "P450_3A4_CSL_Uncertainty",
         "IUPAC_NAME",
         "STATUS_Probability",
         # long list of possible property names within library
         ...
      ],
      "updated_timestamp" : 1446742469199,
      "created_timestamp" : 1446742437871,
      "structures_count" : 108,
      "name" : "My library"
   },
   "metadata" : {
      "comment" : "Some notes."
   }
}
```

However, it seems impractical to send these specific details every time a
client requests general library information and, on the whole,
```service_data``` sounds like a wrong place to store SDF properties.

A new *jsonb* ```library_metadata.index_data``` column would increase data
decoupling.

# Detailed design

Database schema should be updated in order to contain new ```index_data```
column in ```library_metadata``` table. At the simplest, this column would
hold unique properties set for every library table:

```
{
    "properties" : [
       "P450_3A4_CSL_Uncertainty",
       "IUPAC_NAME",
       "STATUS_Probability",
       # long list of possible property names within library
       ...
    ]
}
```

The change should be transparent for the current API (and hence, possible
consumers).

# Further considerations

A separate change to the API should be made to introduce a new method for
retrieving data from the new column. That would allow to remove **properties**
from ```GET /libraries/{library_id}``` response. This change is out of scope of
this RFC, however.
