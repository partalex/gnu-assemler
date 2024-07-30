#include "./include/symbol_list.h"

int main(int argc, char *argv[]) {
    auto *list =
            new SymbolList(
                    "a", new SymbolList(
                            "b", new SymbolList("c")));
    list->log();
    delete list;
    return 0;
}