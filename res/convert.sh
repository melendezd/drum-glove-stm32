#!/bin/sh

out_name='samples'
namespace_name='sample_data'

echo "namespace ${namespace_name} {" > "${out_name}.cpp"
echo "namespace ${namespace_name} {" > "${out_name}.hpp"
for file in *.wav; do
    if [ -f "$file" ]; then
        name="${file%.wav}"

        # convert wav to raw binary of samples
        sox "$file" -t raw "${name}"

        # output declaration to header file
        echo "extern unsigned char *${name};" >> "${out_name}.hpp"

        # output definition to cpp file
        xxd -i "${name}" >> "${out_name}.cpp"

        # remove binary
        rm "${name}"
    fi
done
echo '}' >> "${out_name}.cpp"
echo '}' >> "${out_name}.hpp"

# add attribute
sed -ie 's/unsigned/__attribute__ ((section (".sample_data"))) unsigned/g'  "${out_name}.cpp"
