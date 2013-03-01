// Stores the SVG images.
var gl_svg =
{
    Header:
    {
        search: "M19.625,19.177l-0.432,0.432c-0.529,0.529-1.388,0.529-1.917,0l-4.655-4.655c-0.529-0.529-0.529-1.388,0-1.917l0.432-0.432c0.529-0.529,1.388-0.529,1.917,0l4.655,4.655C20.155,17.79,20.155,18.648,19.625,19.177zM2,11.654C-0.667,8.988-0.667,4.666,2,2s6.988-2.666,9.654,0s2.666,6.988,0,9.654S4.666,14.32,2,11.654zM9.698,3.929c-1.567-1.567-4.109-1.567-5.677,0c-1.567,1.568-1.567,4.109,0,5.677c1.567,1.567,4.109,1.567,5.677,0C11.266,8.039,11.266,5.497,9.698,3.929z",
        settings: "M17.007,8.486c-0.085,0.095-0.13,0.156-0.2,0.237c0.078,0.43,0.128,0.869,0.128,1.322c0,0.446-0.056,0.877-0.131,1.301c0.084,0.099,0.147,0.182,0.256,0.302c0.416,0.465,1.438,0.931,1.7,1.497c0.123,0.263,0.105,0.328-0.061,0.625c-0.135,0.242-1.235,2.148-1.378,2.386c-0.175,0.292-0.222,0.34-0.511,0.366c-0.622,0.054-1.536-0.597-2.146-0.725c-0.151-0.032-0.287-0.06-0.41-0.083c-0.657,0.552-1.408,0.994-2.226,1.299c-0.04,0.114-0.081,0.238-0.127,0.379c-0.194,0.593-0.088,1.71-0.445,2.221c-0.167,0.238-0.231,0.254-0.572,0.26c-0.277,0.005-2.479,0.005-2.755,0c-0.341-0.005-0.406-0.022-0.572-0.26c-0.358-0.511-0.252-1.628-0.446-2.221c-0.046-0.141-0.087-0.265-0.127-0.379c-0.819-0.306-1.573-0.75-2.232-1.304c-0.133,0.024-0.278,0.053-0.446,0.088c-0.61,0.128-1.525,0.779-2.146,0.725c-0.289-0.025-0.336-0.074-0.511-0.366c-0.143-0.238-1.243-2.144-1.378-2.386c-0.166-0.298-0.184-0.362-0.061-0.625c0.263-0.565,1.284-1.032,1.7-1.497c0.125-0.139,0.202-0.238,0.295-0.349c-0.069-0.41-0.126-0.825-0.126-1.254c0-0.438,0.045-0.865,0.118-1.281C2.118,8.671,2.062,8.597,1.962,8.486C1.545,8.022,0.525,7.555,0.261,6.99C0.139,6.727,0.157,6.662,0.323,6.364C0.457,6.122,1.557,4.216,1.7,3.978c0.176-0.292,0.223-0.34,0.512-0.365c0.621-0.055,1.536,0.596,2.146,0.725c0.136,0.029,0.256,0.053,0.369,0.075c0.671-0.564,1.43-1.027,2.271-1.331c0.036-0.103,0.072-0.209,0.112-0.333c0.194-0.593,0.088-1.71,0.446-2.221c0.166-0.238,0.231-0.254,0.572-0.26c0.276-0.004,2.478-0.004,2.755,0c0.341,0.005,0.405,0.022,0.572,0.26c0.357,0.511,0.251,1.628,0.445,2.221c0.04,0.124,0.076,0.23,0.112,0.333c0.838,0.302,1.595,0.763,2.264,1.324c0.104-0.02,0.211-0.042,0.334-0.067c0.61-0.128,1.525-0.779,2.146-0.725c0.289,0.025,0.336,0.073,0.512,0.365c0.143,0.238,1.243,2.144,1.377,2.386c0.166,0.298,0.184,0.363,0.062,0.625C18.445,7.555,17.424,8.022,17.007,8.486zM9.51,5.522c-2.498,0-4.523,2.025-4.523,4.524c0,2.498,2.025,4.523,4.523,4.523s4.523-2.025,4.523-4.523C14.034,7.548,12.008,5.522,9.51,5.522z",
        help: "M8.382,9.836c-1.078,1.11-2.65,2.463-2.555,4.062c-0.009,1.051-0.002,1.158-0.002,1.158l-2.932,0.006c0,0-0.003-1.108,0.003-1.544c0.075-0.532-0.215-1.704,1.806-3.857c0.89-0.872,2.009-2.112,2.276-2.796C7.386,6.06,7.812,2.776,3.61,2.906C1.875,3.007,0.749,3.363,0.003,3.641C0.008,3.014,0,0.634,0,0.634s5.453-1.971,8.764,1.035C11.64,4.835,9.827,8.476,8.382,9.836zM3.482,16.951h1.767c0.498,0,0.901,0.353,0.901,0.789v1.523c0,0.436-0.403,0.789-0.901,0.789H3.482c-0.498,0-0.901-0.353-0.901-0.789V17.74C2.581,17.304,2.984,16.951,3.482,16.951z",
        disconnect: "M9.377,20.01c-5.014,0-9.077-4.066-9.077-9.082c0-3.077,1.532-5.791,3.872-7.434c0.692-0.265,1.305-0.04,1.684,0.438C6.226,4.4,6.378,5.309,5.88,5.711C5.879,5.713,5.877,5.714,5.876,5.716C4.205,6.844,3.104,8.758,3.104,10.93c0,3.47,2.806,6.283,6.268,6.283s6.268-2.813,6.268-6.283c0-2.175-1.104-4.091-2.779-5.218c-0.01-0.009-0.027-0.024-0.035-0.033c-0.418-0.37-0.383-1.276,0.035-1.791c0.403-0.498,1.112-0.672,1.706-0.382c-0.004-0.012-0.021-0.032-0.026-0.044c2.364,1.639,3.916,4.37,3.916,7.466C18.456,15.944,14.391,20.01,9.377,20.01zM9.344,10.04c-0.827,0-1.498-0.692-1.498-1.545v-6.92c0-0.854,0.671-1.545,1.498-1.545s1.497,0.691,1.497,1.545v6.92C10.841,9.348,10.171,10.04,9.344,10.04z"
    },
    Player:
    {
        play: "M0.107,1.671c-0.016,0.622,0,16.068,0,16.649c0,1.174,0.484,1.335,1.461,1.355c0.665,0.014,12.853-8.243,13.476-8.633c0.695-0.436,0.672-1.7,0.008-2.011c-0.666-0.312-12.911-8.71-13.453-8.73C0.778,0.271,0.136,0.569,0.107,1.671z",
        pause: "M14.02,20.033h-1.025c-1.098,0-1.987-0.896-1.987-2V2c0-1.104,0.89-2,1.987-2h1.025c1.098,0,1.988,0.896,1.988,2v16.033C16.007,19.138,15.117,20.033,14.02,20.033zM3.012,20.033H1.987c-1.098,0-1.987-0.896-1.987-2V2c0-1.104,0.89-2,1.987-2h1.025C4.11,0,5,0.896,5,2v16.033C5,19.138,4.11,20.033,3.012,20.033z",
        previous: "M14.006,16.727l-3.362,3.362l-6.7-6.7l0,0L0.6,10.045L10.645,0l3.344,3.344l-6.682,6.683L14.006,16.727z",
        next: "M10.063,13.389L10.063,13.389l-6.7,6.7L0,16.727l6.7-6.7L0.018,3.344L3.362,0l10.044,10.045L10.063,13.389z",
        volume: "M20.847,19.978c-1.523-1.468-0.693-0.6-1.913-1.896c2.025-2.059,3.276-4.899,3.276-8.036c0-3.148-1.261-5.998-3.299-8.06c1.14-1.153,0.921-0.925,1.893-1.92c2.522,2.552,4.084,6.08,4.084,9.979C24.888,13.919,23.345,17.43,20.847,19.978zM17.018,16.19c-1.296-1.257-0.712-0.622-1.926-1.918c1.077-1.076,1.745-2.573,1.745-4.228c0-1.665-0.676-3.17-1.765-4.248C16.499,4.4,15.515,5.396,16.99,3.87c1.572,1.573,2.547,3.758,2.547,6.174C19.537,12.446,18.574,14.619,17.018,16.19zM12.201,20.016c-0.137-0.125-6.382-5.375-6.588-5.501c-0.205-0.126-0.777-0.332-1.063-0.344c-0.286-0.011-2.745,0-3.214,0s-1.315-0.022-1.327-1.098c-0.011-1.075-0.011-4.998,0-6.073C0.02,5.925,0.867,5.902,1.335,5.902s2.928,0.011,3.214,0c0.286-0.012,0.858-0.218,1.063-0.344c0.206-0.125,6.451-5.375,6.588-5.501c0.138-0.126,0.641-0.08,0.641,0.491c0,0.572,0,18.403,0,18.976C12.841,20.096,12.338,20.142,12.201,20.016z",
        mute: "M24.934,12.99l-1.951,1.951l-2.952-2.952l-2.97,2.97l-1.952-1.951l2.97-2.97l-2.97-2.97l1.952-1.952l2.97,2.97l2.938-2.938L24.92,7.1l-2.938,2.938L24.934,12.99zM12.211,20.032c-0.138-0.126-6.388-5.38-6.594-5.506s-0.778-0.332-1.064-0.344c-0.286-0.011-2.747,0-3.216,0c-0.47,0-1.316-0.022-1.328-1.099s-0.012-5.002,0-6.078s0.858-1.099,1.328-1.099c0.469,0,2.93,0.012,3.216,0s0.858-0.218,1.064-0.344s6.456-5.38,6.594-5.506s0.641-0.08,0.641,0.492s0,18.418,0,18.99S12.349,20.158,12.211,20.032z",
        settings: "M4.574,19.67c0,0,7.692-7.582,8.465-8.341c1.774,0.357,2.467,0.096,3.147-0.028c1.335-0.243,4.24-1.854,4.193-5.792c-1.992,1.043-4.159,2.214-4.159,2.214L13.96,6.23l0.006-2.838l4.102-2.203c0,0-3.634-2.933-7.331,0.414C8.711,3.436,8.844,5.816,9.022,7.042c-1.168,1.198-8.354,8.283-8.47,8.426c-0.665,0.822-0.771,2.372-0.089,3.179c0.892,1.054,0.518,0.591,1.02,0.99C1.986,20.037,3.495,20.513,4.574,19.67z",
        repeat: "M26.762,11.53c-0.019,2.841-2.53,5.313-5.116,5.299C20.317,16.821,6.64,16.823,6.64,16.823v3.112c-0.232-0.224-5.181-4.766-5.181-4.766c0.274-0.259,5.181-4.766,5.181-4.766c0.007,0.333,0,3.099,0,3.099s12.938,0.015,14.597,0.007c1.418-0.007,2.138-1.362,2.138-1.867c0-0.581,0.007-2.109,0.007-2.109c0.146-0.164,3.367-3.332,3.367-3.332C26.757,6.465,26.773,9.765,26.762,11.53zM20.326,6.432c0,0-12.937-0.015-14.597-0.007C4.311,6.432,3.591,7.788,3.591,8.293c0,0.581-0.007,2.109-0.007,2.109c-0.146,0.163-3.367,3.331-3.367,3.331C0.208,13.47,0.193,10.17,0.204,8.404c0.019-2.84,2.53-5.313,5.116-5.298c1.329,0.008,15.006,0.006,15.006,0.006V0c0.232,0.224,5.181,4.766,5.181,4.766c-0.274,0.258-5.181,4.765-5.181,4.765C20.319,9.197,20.326,6.432,20.326,6.432z",
        repeatOne: "M26.761,11.53c-0.019,2.841-2.53,5.313-5.116,5.299c-1.329-0.008-15.004-0.006-15.004-0.006v3.112c-0.232-0.224-5.18-4.766-5.18-4.766c0.273-0.259,5.18-4.766,5.18-4.766c0.008,0.333,0,3.099,0,3.099s12.936,0.015,14.595,0.007c1.418-0.007,2.138-1.362,2.138-1.867c0-0.581-0.011-2.769-0.011-3.35c0-0.505-0.719-1.861-2.137-1.868c-1.66-0.008-13.836-0.008-15.496,0C4.312,6.432,3.592,7.788,3.592,8.293c0,0.581-0.006,2.109-0.006,2.109c-0.146,0.163-3.367,3.331-3.367,3.331c-0.01-0.264-0.025-3.563-0.014-5.329c0.018-2.84,2.53-5.313,5.115-5.298c1.329,0.008,14.985,0.008,16.314,0c2.585-0.015,5.098,2.458,5.115,5.298C26.762,10.17,26.771,9.765,26.761,11.53z",
        noRepeat: "M0.205,8.404c0.019-2.84,2.531-5.313,5.116-5.298C6.65,3.114,20.325,3.112,20.325,3.112V0c0.232,0.224,5.181,4.766,5.181,4.766c-0.274,0.258-5.181,4.766-5.181,4.766c-0.007-0.333,0-3.099,0-3.099S7.39,6.417,5.73,6.426C4.312,6.433,3.592,7.788,3.592,8.293c0,0.581,0.011,2.769,0.011,3.35c0,0.505,0.719,1.86,2.137,1.868c1.66,0.008,7.95,0,7.95,0l-0.015,3.318c0,0-7.016-0.008-8.344,0c-2.586,0.015-5.098-2.458-5.116-5.298C0.204,9.766,0.194,10.171,0.205,8.404z",
        random: "M22.224,6.001c0,0-2.282,0-3.574,0c-1.293,0-2.637,2.405-2.637,2.405s-3.563,5.678-3.848,6.028c-1.023,1.629-3.159,2.54-4.341,2.556c-1.135,0.015-7.811,0.023-7.811,0.023V13.99c0,0,6.901,0,7.78,0s2.065-1.641,2.065-1.641s4.246-6.627,4.215-6.704c1.312-2.063,3.638-2.627,4.265-2.627c0.603-0.001,3.885-0.028,3.885-0.028V0l4.809,4.463l-4.809,4.51V6.001zM8.243,6.002c-0.879,0-8.243,0-8.243,0V2.991c0,0,7.14-0.007,8.273-0.007c1.135,0,3.188,0.706,3.743,1.923c0.008,0-1.707,2.672-1.707,2.672S9.122,6.002,8.243,6.002zM19.099,14c1.292,0,3.125,0,3.125,0v-3.004l4.809,4.479l-4.809,4.526V17.01c0,0-3.538,0-3.605,0c-1.007-0.195-2.697-0.652-4.095-2.65c0.031-0.077,1.916-2.659,1.916-2.659S17.807,14,19.099,14z",
        linear: "M22.224,6.001L0,6.002V2.991L22.224,2.99V0l4.809,4.463l-4.809,4.51V6.001zM22.224,10.996l4.809,4.479l-4.809,4.526V17.01l-22.21,0.004V13.99L22.224,14V10.996z",
        fullScreen: "M17.976,19.987v-2.973h4.006v-3.927h3.011v6.899H17.976zM21.981,3.085h-4.006V0h7.017v7.049h-3.011V3.085zM6.998,3.085H2.973v3.964H0V0h6.998V3.085zM2.973,17.015h4.025v2.973H0v-6.899h2.973V17.015z",
        normalScreen: "M20.984,15.982v4.005h-3.009v-6.899h7.017v2.895H20.984zM17.976,0h3.009v4.018h4.008v3.031h-7.017V0zM3.988,15.982H0v-2.895h6.998v6.899h-3.01V15.982zM0,4.018h3.988V0h3.01v7.049H0V4.018z",
        tabLeft: "M31.92,0l0.01,24.248c0,0-3.592-0.018-5.905-0.018C16.183,24.247,7.998,24.247,1.9,24.251c-3.033,0-1.97-0.191-0.038-0.277c6.09,0.041,9.854-6.41,12.037-12.039c2.136-5.509,5.999-11.922,12.115-11.922C27.857,0.013,31.92,0,31.92,0z",
        tabRight: "M5.917,0.013c6.116,0,9.979,6.413,12.115,11.922c2.182,5.629,5.946,12.08,12.037,12.039c1.932,0.086,2.995,0.277-0.038,0.277c-6.098-0.004-14.283-0.004-24.125-0.021C3.592,24.23,0,24.248,0,24.248L0.01,0C0.01,0,4.073,0.013,5.917,0.013z",
        tabClose: "M6.909,0.976L4.415,3.471l2.471,2.471L5.941,6.886L3.471,4.414L0.978,6.907L0.004,5.935l2.493-2.493L0,0.943L0.944,0l2.498,2.498l2.495-2.495L6.909,0.976z",
        add: "M7.995,4.914H4.797v3.202h-1.61V4.914H0V3.208h3.187V0h1.61v3.208h3.198V4.914z",
        addTab: "M24.167,23.961c-6.091,0.041-9.854-6.41-12.037-12.039C9.995,6.413,6.132,0,0.016,0c-0.783,0,27.217,0,28,0c6.116,0,9.979,6.413,12.115,11.922c2.182,5.629,5.946,12.08,12.036,12.039C58.136,23.922,30.136,23.922,24.167,23.961z",
        pin: "M14.562,5.704c-0.482,0.481-1.264,0.481-1.746,0L9.219,2.107c-0.482-0.482-0.482-1.264,0-1.746c0.481-0.481,1.264-0.481,1.745,0l3.598,3.597C15.043,4.44,15.043,5.221,14.562,5.704zM11.984,5.549L9.369,9.591L5.275,5.499l4.043-2.615L11.984,5.549zM9.604,12.338c-0.482,0.482-1.264,0.482-1.746,0L2.529,7.009c-0.481-0.482-0.481-1.264,0-1.746c0.482-0.481,1.264-0.481,1.746,0l5.329,5.33C10.087,11.075,10.087,11.857,9.604,12.338zM5.408,10.276l-5.09,4.577L0,14.535l4.574-5.092L5.408,10.276z",
        seek: "M35.646,9.95L25.537,20.081L15.386,9.95H0V0h49.999v9.95H35.646z"
    },
    ResourceFiles:
    {
        image: "M11.974,10h-23.948c-0.552,0-1.526-0.974-1.526-1.526V-8.474c0-0.552,0.974-1.526,1.526-1.526h23.948c0.552,0,1.526,0.974,1.526,1.526V8.474C13.5,9.026,12.525,10,11.974,10zM11.5-8h-23V7.938h23V-8zM-7.796-3.171c-0.005-0.13-0.157-0.244-0.296-0.269S-8.311-3.647-7.99-3.767C-7.497-3.95-7.212-3.704-7.082-3.86c0.117-0.141,0.045-0.434,0.212-0.573c3.025-2.521,8.073-1.77,8.073-1.77c-4.396,0.55-7.299,2.925-6.849,3.136c0.45,0.211,1.929,1.084,2.149,1.296c0.22,0.212,0.015,0.391-0.349,0.339C-4.21-1.484-5.806-2.507-6.285-2.479C-6.87-2.445-6.882-2.435-7.457-1.486c-0.808,1.333-0.779,3.335-0.971,4.55C-9.03,1.791-8.748-2.051-8.666-2.424C-8.583-2.797-7.792-3.041-7.796-3.171zM-6.05,2.467c0.516,0,1.764,1.506,2.108,1.506s4.388-5.378,4.948-5.378c0.559,0,0.473,1.162,1.291,1.162c0.816,0,2.666-3.399,3.527-3.356c0.859,0.043,4.646,6.97,4.646,6.97l0.043,3.571L-8.502,6.984C-8.502,6.984-6.566,2.467-6.05,2.467z",
        audio: "M9.996,4.847c0.006,0.072,0.027,0.142,0.027,0.214c0,0.076-0.02,0.147-0.027,0.222v0.702H9.823C9.347,7.151,8.011,8.026,6.339,8.026c-2.092,0-4.362-1.047-4.362-2.648s2.204-3.368,4.296-3.368c0.629,0,1.209,0.143,1.726,0.37v-8.213l-9.99,1.478V6.955c0.002,0.036,0.014,0.071,0.014,0.107c0,0.038-0.012,0.073-0.014,0.111v0.591h-0.102c-0.387,1.277-1.791,2.262-3.568,2.262c-2.092,0-4.362-1.047-4.362-2.648s2.203-3.368,4.296-3.368c0.635,0,1.22,0.146,1.741,0.376v-8.447l-0.002,0v-3.896l13.987-2.069v3.896l-0.002,0V4.847z",
        video: "M8.776,10.023H-8.776c-0.738,0-1.725-0.987-1.725-1.725V2.064h20.984L10.5,8.298C10.5,9.036,9.514,10.023,8.776,10.023zM8.495,4.009H-8.5v1.083H8.495V4.009zM8.495,7.047H-8.5v1.007H8.495V7.047zM9.513-1.958h0.88v3.005H6.507L9.513-1.958zM5.639-9.243l3.806-0.78l0.604,2.944L9.187-6.902L5.639-9.243zM5.878-1.958L2.873,1.047h-3.078L2.8-1.958H5.878zM-0.937-7.894l3.015-0.618l3.548,2.34L2.611-5.554L-0.937-7.894zM-0.835-1.958l-3.005,3.005h-3.061l3.005-3.005H-0.835zM-7.497-6.549l2.999-0.615l3.548,2.34l-2.999,0.615L-7.497-6.549zM-10.496,1.007v-2.965h2.965L-10.496,1.007zM-10.496-3.205v-2.244l2.986,1.97l-2.92,0.599L-10.496-3.205z",
        document: "M8.003,9.966L-8.003,9.974l0-12.943l7.06-7.005l8.947,0.04L8.003,9.966zM6.005-8.042H0.017l-0.038,6.063L-6.017-2.01v9.994H6.005V-8.042z",
        other: "M8.455,1.767H6.267c-0.135,0.481-0.339,0.93-0.572,1.36l1.59,1.59c0.616,0.616,0.616,1.615,0,2.231L6.969,7.262c-0.616,0.616-1.615,0.616-2.231,0L3.186,5.709C2.768,5.947,2.318,6.133,1.85,6.276v2.172c0,0.871-0.706,1.577-1.577,1.577h-0.446c-0.871,0-1.577-0.706-1.577-1.577V6.304C-2.244,6.168-2.703,5.96-3.143,5.721L-4.7,7.277c-0.616,0.616-1.615,0.616-2.23,0l-0.315-0.315c-0.616-0.616-0.616-1.615,0-2.23l1.521-1.521C-5.969,2.78-6.16,2.317-6.304,1.834h-2.151c-0.871,0-1.578-0.706-1.578-1.577v-0.446c0-0.871,0.707-1.577,1.578-1.577h2.145c0.14-0.491,0.341-0.954,0.585-1.39l-1.56-1.56c-0.616-0.616-0.616-1.615,0-2.23l0.315-0.315c0.616-0.616,1.615-0.616,2.231,0l1.567,1.567c0.416-0.23,0.854-0.425,1.32-0.561v-2.191c0-0.871,0.707-1.578,1.578-1.578h0.446c0.871,0,1.577,0.707,1.577,1.578v2.173c0.484,0.136,0.935,0.343,1.368,0.58L4.7-7.277c0.616-0.616,1.615-0.616,2.23,0l0.315,0.315c0.616,0.616,0.616,1.615,0,2.231L5.693-3.178c0.238,0.42,0.423,0.873,0.564,1.344h2.197c0.872,0,1.578,0.706,1.578,1.577v0.446C10.032,1.06,9.326,1.767,8.455,1.767zM-0.021-3.808c-2.108,0-3.817,1.709-3.817,3.817c0,2.108,1.709,3.818,3.817,3.818S3.796,2.118,3.796,0.01C3.796-2.098,2.087-3.808-0.021-3.808z",
        folder: "M7,6.984H-7c-1.104,0-2-0.896-2-2V0.966L9,0.904v4.081C9,6.089,8.104,6.984,7,6.984zM8.936-1.111L-9-1.074v-3.911c0-1.104,0.896-2,2-2h4c1.104,0,1.967,0.894,2,2c-0.026,0.63,0.404,0.942,1.008,0.976c0.604,0.033,6.538-0.039,6.961-0.039C8.108-4.016,9-3.124,9-2.062C8.84-2.064,8.936-1.111,8.936-1.111z",
        resize: "M4.99,7.031V7.01H1.988V5H4.99V1.957H7V5v2.01v0.021H4.99zM4.759-3.303L1.711-0.289L0.283-1.715l3.033-3L1-7l6,0.031L6.969-1.031L4.759-3.303zM-4.99-1.957H-7V-5v-2.01v-0.021h2.01v0.021h3.002V-5H-4.99V-1.957zM-4.76,3.272l3.042-3.008l1.427,1.427l-3.025,2.992L-1,6.969l-6-0.031L-6.969,1L-4.76,3.272z",
        list: "M-9.011,6.989V5.022H9.011v1.966H-9.011zM-9.011,1.021H9.011v1.966H-9.011V1.021zM-9.011-2.978H9.011v1.966H-9.011V-2.978zM-9.011-6.989H9.011v1.966H-9.011V-6.989z",
        hierarchy: "M-9.011-5.023v-1.966H9.011v1.966H-9.011zM9.008-1.012H-5.014v-1.966H9.008V-1.012zM9.005,1.021v1.966H-1.007V1.021H9.005zM9.008,6.989H-5.009V5.022H9.008V6.989z",
        split: "M3.012,6.962V4.996h5.992v1.966H3.012zM3.012,0.996h5.992v1.966H3.012V0.996zM3.012-3.004h5.992v1.966H3.012V-3.004zM3.012-7.004h5.992v1.966H3.012V-7.004zM-1.002-6.979h1.996V7.004h-1.996V-6.979zM-9.004,4.996h5.988v1.966h-5.988V4.996zM-9.004,0.996h5.988v1.966h-5.988V0.996zM-9.004-3.004h5.988v1.966h-5.988V-3.004zM-9.004-7.004h5.988v1.966h-5.988V-7.004z",
        block: "M0.999,7.061V0.939h8.003v6.121H0.999zM7.018,3.041H3.013v1.976h4.005V3.041zM0.999-7.061h8.003v6.121H0.999V-7.061zM3.013-2.983h4.005v-1.976H3.013V-2.983zM-9.001,0.939h8.003v6.121h-8.003V0.939zM-6.987,5.017h4.005V3.041h-4.005V5.017zM-9.001-7.061h8.003v6.121h-8.003V-7.061zM-6.987-2.983h4.005v-1.976h-4.005V-2.983z",
        add: "M4.999,1.042H1.016v3.954h-2.022V1.042h-3.988v-2.085h3.988v-3.954h2.022v3.954h3.982V1.042z"
    },
    TaskButtons:
    {
        close: "M4.468,2.94L2.942,4.466L0,1.523l-2.942,2.942L-4.468,2.94l2.942-2.942l-2.938-2.939l1.525-1.525L0-1.527l2.938-2.938l1.525,1.525L1.525-0.002L4.468,2.94z",
        hide: "M-5.003-1.523H5.003v3.045H-5.003V-1.523z",
        window: "M-5,4.505v-9.011H5v9.011H-5zM2.992-2.506h-5.978v5.021h5.978V-2.506z",
        fullscreen: "M4.002,1.052l-3-0.006v2.951h-2V1.042l-3.004-0.006v-2.063l3.004,0.006v-2.977h2v2.98l3,0.006V1.052z"
    },
    Window:
    {
        close: "M8.946,7.407L7.421,8.932L4.479,5.99L1.525,8.943L0,7.418l2.954-2.954L0.015,1.525L1.54,0l2.938,2.939l2.929-2.928l1.524,1.525L6.004,4.464L8.946,7.407z",
        hide: "M0,0h10.006v3.045H0V0z",
        window: "M0,10.011V0h10v10.011H0zM7.992,2H2.015v6.021h5.977V2z",
        fullscreen: "M9.989,6.029l-4,0.006v3.964h-2V6.038L0,6.044V3.982l3.989-0.006V0h2v3.972l4-0.006V6.029z"
    }
};