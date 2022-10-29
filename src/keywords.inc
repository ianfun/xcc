Token getKeyword(const char *s){
	switch(*s++){
		case 'a':
		switch(*s++){
			case 'u':
			switch(*s++){
				case 't':
				switch(*s++){
					case 'o':
					switch(*s++){
						case '\0': return Kauto;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'f':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 'r':
				switch(*s++){
					case '\0': return Kfor;
					default: return TNul;
				}
				default: return TNul;
			}
			case 'l':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 't':
						switch(*s++){
							case '\0': return Kfloat;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'g':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 't':
				switch(*s++){
					case 'o':
					switch(*s++){
						case '\0': return Kgoto;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'i':
		switch(*s++){
			case 'n':
			switch(*s++){
				case 'l':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 'n':
						switch(*s++){
							case 'e':
							switch(*s++){
								case '\0': return Kinline;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 't':
				switch(*s++){
					case '\0': return Kint;
					default: return TNul;
				}
				default: return TNul;
			}
			case 'f':
			switch(*s++){
				case '\0': return Kif;
				default: return TNul;
			}
			default: return TNul;
		}
		case 'c':
		switch(*s++){
			case 'h':
			switch(*s++){
				case 'a':
				switch(*s++){
					case 'r':
					switch(*s++){
						case '\0': return Kchar;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'o':
			switch(*s++){
				case 'n':
				switch(*s++){
					case 't':
					switch(*s++){
						case 'i':
						switch(*s++){
							case 'n':
							switch(*s++){
								case 'u':
								switch(*s++){
									case 'e':
									switch(*s++){
										case '\0': return Kcontinue;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					case 's':
					switch(*s++){
						case 't':
						switch(*s++){
							case '\0': return Kconst;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'e':
		switch(*s++){
			case 'n':
			switch(*s++){
				case 'u':
				switch(*s++){
					case 'm':
					switch(*s++){
						case '\0': return Kenum;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'x':
			switch(*s++){
				case 't':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'r':
						switch(*s++){
							case 'n':
							switch(*s++){
								case '\0': return Kextern;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'l':
			switch(*s++){
				case 's':
				switch(*s++){
					case 'e':
					switch(*s++){
						case '\0': return Kelse;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'l':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 'n':
				switch(*s++){
					case 'g':
					switch(*s++){
						case '\0': return Klong;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'r':
		switch(*s++){
			case 'e':
			switch(*s++){
				case 'g':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 's':
						switch(*s++){
							case 't':
							switch(*s++){
								case 'e':
								switch(*s++){
									case 'r':
									switch(*s++){
										case '\0': return Kregister;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 's':
				switch(*s++){
					case 't':
					switch(*s++){
						case 'r':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'c':
								switch(*s++){
									case 't':
									switch(*s++){
										case '\0': return Krestrict;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 't':
				switch(*s++){
					case 'u':
					switch(*s++){
						case 'r':
						switch(*s++){
							case 'n':
							switch(*s++){
								case '\0': return Kreturn;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 's':
		switch(*s++){
			case 'h':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'r':
					switch(*s++){
						case 't':
						switch(*s++){
							case '\0': return Kshort;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'i':
			switch(*s++){
				case 'z':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'o':
						switch(*s++){
							case 'f':
							switch(*s++){
								case '\0': return Ksizeof;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'g':
				switch(*s++){
					case 'n':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 'd':
							switch(*s++){
								case '\0': return Ksigned;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 't':
			switch(*s++){
				case 'a':
				switch(*s++){
					case 't':
					switch(*s++){
						case 'i':
						switch(*s++){
							case 'c':
							switch(*s++){
								case '\0': return Kstatic;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'd':
		switch(*s++){
			case 'e':
			switch(*s++){
				case 'f':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 'u':
						switch(*s++){
							case 'l':
							switch(*s++){
								case 't':
								switch(*s++){
									case '\0': return Kdefault;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'o':
			switch(*s++){
				case 'u':
				switch(*s++){
					case 'b':
					switch(*s++){
						case 'l':
						switch(*s++){
							case 'e':
							switch(*s++){
								case '\0': return Kdouble;
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case '\0': return Kdo;
				default: return TNul;
			}
			default: return TNul;
		}
		case 'v':
		switch(*s++){
			case 'o':
			switch(*s++){
				case 'l':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 't':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'l':
								switch(*s++){
									case 'e':
									switch(*s++){
										case '\0': return Kvolatile;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 'i':
				switch(*s++){
					case 'd':
					switch(*s++){
						case '\0': return Kvoid;
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 't':
		switch(*s++){
			case 'y':
			switch(*s++){
				case 'p':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'd':
						switch(*s++){
							case 'e':
							switch(*s++){
								case 'f':
								switch(*s++){
									case '\0': return Ktypedef;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'u':
		switch(*s++){
			case 'n':
			switch(*s++){
				case 'i':
				switch(*s++){
					case 'o':
					switch(*s++){
						case 'n':
						switch(*s++){
							case '\0': return Kunion;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 's':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 'g':
						switch(*s++){
							case 'n':
							switch(*s++){
								case 'e':
								switch(*s++){
									case 'd':
									switch(*s++){
										case '\0': return Kunsigned;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'b':
		switch(*s++){
			case 'r':
			switch(*s++){
				case 'e':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 'k':
						switch(*s++){
							case '\0': return Kbreak;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case 'w':
		switch(*s++){
			case 'h':
			switch(*s++){
				case 'i':
				switch(*s++){
					case 'l':
					switch(*s++){
						case 'e':
						switch(*s++){
							case '\0': return Kwhile;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		case '_':
		switch(*s++){
			case 'A':
			switch(*s++){
				case 'l':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 'g':
						switch(*s++){
							case 'n':
							switch(*s++){
								case 'a':
								switch(*s++){
									case 's':
									switch(*s++){
										case '\0': return K_Alignas;
										default: return TNul;
									}
									default: return TNul;
								}
								case 'o':
								switch(*s++){
									case 'f':
									switch(*s++){
										case '\0': return K_Alignof;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				case 't':
				switch(*s++){
					case 'o':
					switch(*s++){
						case 'm':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'c':
								switch(*s++){
									case '\0': return K_Atomic;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'B':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'o':
					switch(*s++){
						case 'l':
						switch(*s++){
							case '\0': return K_Bool;
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'D':
			switch(*s++){
				case 'e':
				switch(*s++){
					case 'c':
					switch(*s++){
						case 'i':
						switch(*s++){
							case 'm':
							switch(*s++){
								case 'a':
								switch(*s++){
									case 'l':
									switch(*s++){
										case '1':
										switch(*s++){
											case '2':
											switch(*s++){
												case '8':
												switch(*s++){
													case '\0': return K_Decimal128;
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										case '6':
										switch(*s++){
											case '4':
											switch(*s++){
												case '\0': return K_Decimal64;
												default: return TNul;
											}
											default: return TNul;
										}
										case '3':
										switch(*s++){
											case '2':
											switch(*s++){
												case '\0': return K_Decimal32;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'C':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'm':
					switch(*s++){
						case 'p':
						switch(*s++){
							case 'l':
							switch(*s++){
								case 'e':
								switch(*s++){
									case 'x':
									switch(*s++){
										case '\0': return K_Complex;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'I':
			switch(*s++){
				case 'm':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 'g':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'n':
								switch(*s++){
									case 'a':
									switch(*s++){
										case 'r':
										switch(*s++){
											case 'y':
											switch(*s++){
												case '\0': return K_Imaginary;
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'N':
			switch(*s++){
				case 'o':
				switch(*s++){
					case 'r':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 't':
							switch(*s++){
								case 'u':
								switch(*s++){
									case 'r':
									switch(*s++){
										case 'n':
										switch(*s++){
											case '\0': return K_Noreturn;
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'S':
			switch(*s++){
				case 't':
				switch(*s++){
					case 'a':
					switch(*s++){
						case 't':
						switch(*s++){
							case 'i':
							switch(*s++){
								case 'c':
								switch(*s++){
									case '_':
									switch(*s++){
										case 'a':
										switch(*s++){
											case 's':
											switch(*s++){
												case 's':
												switch(*s++){
													case 'e':
													switch(*s++){
														case 'r':
														switch(*s++){
															case 't':
															switch(*s++){
																case '\0': return K_Static_assert;
																default: return TNul;
															}
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'T':
			switch(*s++){
				case 'h':
				switch(*s++){
					case 'r':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 'a':
							switch(*s++){
								case 'd':
								switch(*s++){
									case '_':
									switch(*s++){
										case 'l':
										switch(*s++){
											case 'o':
											switch(*s++){
												case 'c':
												switch(*s++){
													case 'a':
													switch(*s++){
														case 'l':
														switch(*s++){
															case '\0': return K_Thread_local;
															default: return TNul;
														}
														default: return TNul;
													}
													default: return TNul;
												}
												default: return TNul;
											}
											default: return TNul;
										}
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case '_':
			switch(*s++){
				case 'a':
				switch(*s++){
					case 's':
					switch(*s++){
						case 'm':
						switch(*s++){
							case '_':
							switch(*s++){
								case '_':
								switch(*s++){
									case '\0': return K__asm__;
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			case 'G':
			switch(*s++){
				case 'e':
				switch(*s++){
					case 'n':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 'r':
							switch(*s++){
								case 'i':
								switch(*s++){
									case 'c':
									switch(*s++){
										case '\0': return K_Generic;
										default: return TNul;
									}
									default: return TNul;
								}
								default: return TNul;
							}
							default: return TNul;
						}
						default: return TNul;
					}
					default: return TNul;
				}
				default: return TNul;
			}
			default: return TNul;
		}
		default: return TNul;
	}
}
