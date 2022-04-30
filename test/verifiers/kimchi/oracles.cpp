//---------------------------------------------------------------------------//
// Copyright (c) 2021-2022 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2021-2022 Nikita Kaskov <nbering@nil.foundation>
// Copyright (c) 2022 Ilia Shirobokov <i.shirobokov@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE blueprint_plonk_kimchi_oracles_test

#include <boost/test/unit_test.hpp>

#include <nil/crypto3/algebra/curves/vesta.hpp>
#include <nil/crypto3/algebra/fields/arithmetic_params/vesta.hpp>
#include <nil/crypto3/algebra/random_element.hpp>

#include <nil/crypto3/hash/algorithm/hash.hpp>
#include <nil/crypto3/hash/sha2.hpp>
#include <nil/crypto3/hash/keccak.hpp>

#include <nil/crypto3/zk/snark/arithmetization/plonk/params.hpp>

#include <nil/crypto3/zk/blueprint/plonk.hpp>
#include <nil/crypto3/zk/assignment/plonk.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/oracles_scalar.hpp>
#include <nil/crypto3/zk/components/systems/snark/plonk/kimchi/detail/verifier_index.hpp>

#include "test_plonk_component.hpp"
#include "proof_data.hpp"

using namespace nil::crypto3;

BOOST_AUTO_TEST_SUITE(blueprint_plonk_oracles_test_suite)

BOOST_AUTO_TEST_CASE(blueprint_plonk_oracles_test) {

    using curve_type = algebra::curves::vesta;
    using BlueprintFieldType = typename curve_type::scalar_field_type;
    constexpr std::size_t WitnessColumns = 15;
    constexpr std::size_t PublicInputColumns = 1;
    constexpr std::size_t ConstantColumns = 1;
    constexpr std::size_t SelectorColumns = 10;
    using ArithmetizationParams = zk::snark::plonk_arithmetization_params<WitnessColumns,
        PublicInputColumns, ConstantColumns, SelectorColumns>;
    using ArithmetizationType = zk::snark::plonk_constraint_system<BlueprintFieldType,
                ArithmetizationParams>;
    using AssignmentType = zk::blueprint_assignment_table<ArithmetizationType>;
    using hash_type = nil::crypto3::hashes::keccak_1600<256>;
    constexpr std::size_t Lambda = 40;

    using component_type = zk::components::oracles_scalar<ArithmetizationType, curve_type,
                                                            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14>;
    using var = zk::snark::plonk_variable<BlueprintFieldType>;

    zk::snark::pickles_proof<curve_type> kimchi_proof = test_proof();

    typename BlueprintFieldType::value_type joint_combiner = 0;
    typename BlueprintFieldType::value_type beta = 0;
    typename BlueprintFieldType::value_type gamma = 0;
    typename BlueprintFieldType::value_type alpha = 0x0000000000000000000000000000000005321CB83A4BCD5C63F489B5BF95A8DC_cppui256;
    typename BlueprintFieldType::value_type zeta = 0x0000000000000000000000000000000062F9AE3696EA8F0A85043221DE133E32_cppui256;
    typename BlueprintFieldType::value_type fq_digest = 0x01D4E77CCD66755BDDFDBB6E4E8D8D17A6708B9CB56654D12070BD7BF4A5B33B_cppui256;
    typename BlueprintFieldType::value_type expected_alpha = 0x23A8600917236F0E644D49DD5E6CA89537CE3047DA7E29D2A7B8CA6006616092_cppui256;
    std::cout<<"Expected alpha: "<<expected_alpha.data<<std::endl;
    typename BlueprintFieldType::value_type expected_zeta = 0x3D0F1F3A3D07DC73FBDF3718FFE270122AA367FB5BA667AD4A4AB81167D21BE4_cppui256;
    std::cout<<"Expected zeta: "<<expected_zeta.data<<std::endl;

    zk::components::kimchi_verifier_index_scalar<curve_type> verifier_index;
    verifier_index.w = 0x1B1A85952300603BBF8DD3068424B64608658ACBB72CA7D2BB9694ADFA504418_cppui256;
    verifier_index.max_poly_size = 512;
    verifier_index.zkpm = {0x2C46205451F6C3BBEA4BABACBEE609ECF1039A903C42BFF639EDC5BA33356332_cppui256,
        0x1764D9CB4C64EBA9A150920807637D458919CB6948821F4D15EB1994EADF9CE3_cppui256,
        0x0140117C8BBC4CE4644A58F7007148577782213065BB9699BF5C391FBE1B3E6D_cppui256,
        0x0000000000000000000000000000000000000000000000000000000000000001_cppui256};
    verifier_index.n = 512;
    verifier_index.public_input_size = 0;
    verifier_index.alpha_powers = 1;

    zk::components::kimchi_proof_scalar<curve_type> proof;
    typename component_type::params_type::fq_sponge_output fq_output = {
        var(0, 0, false, var::column_type::public_input), var(0, 1, false, var::column_type::public_input), 
        var(0, 2, false, var::column_type::public_input), var(0, 3, false, var::column_type::public_input),
        var(0, 4, false, var::column_type::public_input), var(0, 5, false, var::column_type::public_input) 
    };

    typename component_type::params_type params = {verifier_index, proof, fq_output};
    std::vector<typename BlueprintFieldType::value_type> public_input = {joint_combiner, beta, gamma, 
        alpha, zeta, fq_digest};

    auto result_check = [](AssignmentType &assignment, 
        component_type::result_type &real_res) {
    };

    test_component<component_type, BlueprintFieldType, ArithmetizationParams, hash_type, Lambda> (params, public_input, result_check);
}

BOOST_AUTO_TEST_SUITE_END()