SE350 RTOS
==========

The best OS ever!

Naming Conventions
------------------

Try to follow the existing code as much as possible. Most of it's following
more or less the same conventions now.

 - Types: **LikeThis**
 - Exported functions: **source_file_or_conceptual_unit_then_name**
 - Local functions: **whatever_you_want** (must have static linkage! Short names are fine here (1-2 words))
 - Local variables: **like_this_again** (should have short names! :)
 - File global variables: **s_like_this_with_a_pretty_long_name** (must have static linkage, and should have fairly long names- typically several words.)
 - Project global variables: Don't. Ever.
 - Constants/enum values: **LIKE_THIS** (note, however, that the enum name is a type, and so should be LikeThis)

