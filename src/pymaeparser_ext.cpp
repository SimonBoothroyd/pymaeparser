#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>

#include <boost/dynamic_bitset.hpp>

#include <maeparser/MaeBlock.hpp>
#include <maeparser/MaeConstants.hpp>
#include <maeparser/Reader.hpp>
#include <maeparser/Writer.hpp>

namespace nb = nanobind;


/**
 * @brief Converts an indexed property list to a Python list
 * @tparam T The type of property (uint8_t, int, double, or std::string)
 * @param props The indexed property to convert
 * @param block_size The size of the block containing the properties
 * @return A Python list containing the property values, with None for undefined values
 */
template<typename T>
nb::list convert_indexed_properties(const std::shared_ptr<schrodinger::mae::IndexedProperty<T> > &props,
                                    const size_t block_size) {
    nb::list result;
    for (size_t i = 0; i < block_size; ++i) {
        if (props->isDefined(i)) {
            auto value = props->at(i);

            if constexpr (std::is_same_v<T, uint8_t>) {
                result.append(static_cast<bool>(value));
            } else {
                result.append(value);
            }
        } else {
            result.append(nb::none());
        }
    }
    return result;
}

/**
 * @brief Adds all properties of a given type to a Python dictionary
 * @tparam T The type of properties to add (uint8_t, int, double, or std::string)
 * @param dict The Python dictionary to add properties to
 * @param props Map of property names to property values
 * @param block_size The size of the block containing the properties
 */
template<typename T>
void add_properties_to_dict(nb::dict &dict,
                            const std::map<std::string, std::shared_ptr<schrodinger::mae::IndexedProperty<T> > > &props,
                            size_t block_size) {
    for (const auto &[key, value]: props) {
        dict[key.c_str()] = convert_indexed_properties(value, block_size);
    }
}

/**
 * @brief Processes all property types for a block and adds them to a Python dictionary
 * @param dict The Python dictionary to add properties to
 * @param block The indexed block containing the properties
 */
void process_block_properties(nb::dict &dict,
                              const std::shared_ptr<const schrodinger::mae::IndexedBlock> &block) {
    add_properties_to_dict(dict, block->getProperties<uint8_t>(), block->size());
    add_properties_to_dict(dict, block->getProperties<int>(), block->size());
    add_properties_to_dict(dict, block->getProperties<double>(), block->size());
    add_properties_to_dict(dict, block->getProperties<std::string>(), block->size());
}

/**
 * @brief Reads an MAE file and extracts structure information
 * @param filename Path to the MAE file to read
 * @return Vector of Python dictionaries, each containing information about a structure:
 *         - title: Structure title (if present)
 *         - props: Dictionary of structure properties
 *         - atoms: Dictionary of atom properties (if present)
 *         - bonds: Dictionary of bond properties (if present)
 */
std::vector<nb::dict> read_mae(const std::string &filename) {
    schrodinger::mae::Reader reader(filename);
    std::vector<nb::dict> structures;

    while (const auto block = reader.next(schrodinger::mae::CT_BLOCK)) {
        nb::dict structure;

        if (block->hasStringProperty(schrodinger::mae::CT_TITLE)) {
            structure["title"] = block->getStringProperty(schrodinger::mae::CT_TITLE);
        }

        nb::dict props;
        for (const auto &[k, v]: block->getProperties<uint8_t>()) { props[k.c_str()] = bool(v); }
        for (const auto &[k, v]: block->getProperties<int>()) { props[k.c_str()] = v; }
        for (const auto &[k, v]: block->getProperties<double>()) { props[k.c_str()] = v; }
        for (const auto &[k, v]: block->getProperties<std::string>()) { props[k.c_str()] = v; }

        if (props.contains(schrodinger::mae::CT_TITLE)) {
            nb::del(props[schrodinger::mae::CT_TITLE]);
        }
        structure["props"] = props;

        if (const auto atom_block = block->getIndexedBlock(schrodinger::mae::ATOM_BLOCK)) {
            nb::dict atoms;
            process_block_properties(atoms, atom_block);
            structure["atoms"] = atoms;
        }
        if (const auto bond_block = block->getIndexedBlock(schrodinger::mae::BOND_BLOCK)) {
            nb::dict bonds;
            process_block_properties(bonds, bond_block);
            structure["bonds"] = bonds;
        }

        structures.push_back(structure);
    }

    return structures;
}


/**
 * @brief Adds all properties from a Python dictionary to a MAE block
 * @param block The MAE block to add properties to
 * @param props The Python dictionary containing properties
 * @throws std::runtime_error If a property has an unsupported type
 */
void add_properties_to_block(const std::shared_ptr<schrodinger::mae::Block> &block, const nb::dict &props) {
    for (const auto &item: props) {
        auto key = nb::cast<std::string>(item.first);
        nb::handle value = item.second;

        if (key.compare(0, 2, "i_") == 0) {
            block->setIntProperty(key, nb::cast<int>(value));
        } else if (key.compare(0, 2, "r_") == 0) {
            block->setRealProperty(key, nb::cast<double>(value));
        } else if (key.compare(0, 2, "s_") == 0) {
            block->setStringProperty(key, nb::cast<std::string>(value));
        } else if (key.compare(0, 2, "b_") == 0) {
            block->setBoolProperty(key, nb::cast<uint8_t>(value));
        } else {
            throw std::runtime_error("Unsupported property type for key: " + key);
        }
    }
}

/**
 * @brief Creates an indexed property in a MAE block from a Python list of values
 * @tparam T The type of property (uint8_t, int, double, or std::string)
 * @param name The name of the property
 * @param values Python list containing the property values
 * @param block_size The size of the block
 * @param block The MAE indexed block to add the property to
 */
template<typename T>
void create_indexed_property(
    const std::string &name,
    const nb::list &values,
    const size_t block_size,
    std::shared_ptr<schrodinger::mae::IndexedBlock> &block
) {
    auto m_values = std::vector<T>();
    m_values.reserve(block_size);

    auto m_is_null = new boost::dynamic_bitset(block_size);

    for (size_t i = 0; i < block_size; ++i) {
        if (values[i].is_none()) {
            m_is_null->set(i);
            m_values.push_back(T());
        } else {
            m_values.push_back(nb::cast<T>(values[i]));
        }
    }

    auto property = std::make_shared<schrodinger::mae::IndexedProperty<T> >(m_values, m_is_null);
    block->setProperty(name, property);
}

/**
 * @brief Adds indexed properties from a Python dictionary to a MAE indexed block
 * @param block The MAE indexed block to add properties to
 * @param props The Python dictionary containing properties
 * @throws std::runtime_error If property lists have inconsistent sizes or if a property has an unsupported type
 */
void add_indexed_properties_to_block(std::shared_ptr<schrodinger::mae::IndexedBlock> &block,
                                     const nb::dict &props) {
    size_t block_size = 0;

    for (const auto &item: props) {
        const auto values = nb::cast<nb::list>(item.second);
        block_size = values.size();
        break;
    }

    if (block_size == 0) { return; }

    for (const auto &item: props) {
        auto key = nb::cast<std::string>(item.first);
        auto values = nb::cast<nb::list>(item.second);

        if (values.size() != block_size) {
            throw std::runtime_error("Inconsistent property list sizes for key: " + key);
        }

        if (key.compare(0, 2, "i_") == 0) {
            create_indexed_property<int>(key, values, block_size, block);
        } else if (key.compare(0, 2, "r_") == 0) {
            create_indexed_property<double>(key, values, block_size, block);
        } else if (key.compare(0, 2, "s_") == 0) {
            create_indexed_property<std::string>(key, values, block_size, block);
        } else if (key.compare(0, 2, "b_") == 0) {
            create_indexed_property<uint8_t>(key, values, block_size, block);
        } else {
            throw std::runtime_error("Unsupported property type for key: " + key);
        }
    }
}

/**
 * @brief Writes structure information to an MAE file
 * @param filename Path to the MAE file to write
 * @param structures List of dictionaries containing structure information:
 *        - title: Structure title (optional)
 *        - props: Dictionary of structure properties (optional)
 *        - atoms: Dictionary of atom properties (optional)
 *        - bonds: Dictionary of bond properties (optional)
 */
void write_mae(const std::vector<nb::dict> &structures, const std::string &filename) {
    schrodinger::mae::Writer writer(filename);

    for (const auto &structure: structures) {
        auto block = std::make_shared<schrodinger::mae::Block>(schrodinger::mae::CT_BLOCK);
        auto block_map = std::make_shared<schrodinger::mae::IndexedBlockMap>();

        if (structure.contains("title")) {
            const auto title = nb::cast<std::string>(structure["title"]);
            block->setStringProperty(schrodinger::mae::CT_TITLE, title);
        }
        if (structure.contains("props")) {
            nb::dict props = structure["props"];
            add_properties_to_block(block, props);
        }
        if (structure.contains("atoms")) {
            auto atom_block = std::make_shared<schrodinger::mae::IndexedBlock>(schrodinger::mae::ATOM_BLOCK);
            nb::dict atoms = structure["atoms"];

            add_indexed_properties_to_block(atom_block, atoms);
            block_map->addIndexedBlock("m_atom", atom_block);
        }
        if (structure.contains("bonds")) {
            auto bond_block = std::make_shared<schrodinger::mae::IndexedBlock>(schrodinger::mae::BOND_BLOCK);
            nb::dict bonds = structure["bonds"];

            add_indexed_properties_to_block(bond_block, bonds);
            block_map->addIndexedBlock("m_bond", bond_block);
        }

        block->setIndexedBlockMap(block_map);

        writer.write(block);
    }
}


/**
 * @brief Python module for reading and writing Maestro MAE files
 * @param m The module object to define functions in
 * @details Provides functionality to read and write MAE files and extract structure information
 *          including atoms, bonds, and global properties
 */
NB_MODULE(pymaeparser_ext, m) {
    m.def("read_mae", &read_mae, "Read an MAE file and return atoms/bonds info");
    m.def("write_mae", &write_mae, "Write an MAE file containing atoms/bonds info");
}
